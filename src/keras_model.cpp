/*
 * Copyright (c) 2016 Robert W. Rose
 *
 * MIT License, see LICENSE file.
 */

#include "learnedindex/keras_model.h"

#include <cmath>
#include <fstream>
#include <limits>
#include <stdio.h>
#include <utility>
#include <iostream> 

bool ReadUnsignedInt(std::ifstream* file, unsigned int* i) {
    file->read((char*)i, sizeof(unsigned int));

    return true;
}

bool ReadFloat(std::ifstream* file, float* f) {
    KASSERT(file, "Invalid file stream");
    KASSERT(f, "Invalid pointer");

    file->read((char*)f, sizeof(float));
    KASSERT(file->gcount() == sizeof(float), "Expected float");

    return true;
}

bool ReadFloats(std::ifstream* file, float* f, size_t n) {
    KASSERT(file, "Invalid file stream");
    KASSERT(f, "Invalid pointer");

    file->read((char*)f, sizeof(float) * n);
    KASSERT(((unsigned int)file->gcount()) == sizeof(float) * n,
            "Expected floats");

    return true;
}

bool KerasLayerActivation::LoadLayer(std::ifstream* file) {
    unsigned int activation = 0;
    ReadUnsignedInt(file, &activation);

    if(activation == kRelu)
        activation_type_ = kRelu;
    else if(activation == kSoftMax)
        activation_type_ = kSoftMax;
    else activation_type_ = kLinear;
    return true;
}

bool KerasLayerActivation::Apply(Tensor* in, Tensor* out) {
    *out = *in;

    if(activation_type_ == kRelu){
        for (size_t i = 0; i < out->data_.size(); i++) {
            //std::cout << out->data_[i] << std::endl;
            if (out->data_[i] < 0.0) {
                out->data_[i] = 0.0;
            }
        }
    }
    else if(activation_type_ == kSoftMax){
        auto maxElementIndex = std::max_element(out->data_.begin(),out->data_.end()) - out->data_.begin();
        float max_element = out->data_[maxElementIndex];
        double data_sum = 0.0;
        for(size_t i = 0; i < out->data_.size(); ++i){
            out->data_[i] = std::exp(out->data_[i] - max_element);
            data_sum += out->data_[i];
        }
        for(size_t i = 0; i < out->data_.size(); ++i){
            out->data_[i] /= data_sum;
        }
    }

    return true;
}


bool KerasLayerDense::LoadLayer(std::ifstream* file) {
    KASSERT(file, "Invalid file stream");

    unsigned int weights_rows = 0;
    KASSERT(ReadUnsignedInt(file, &weights_rows), "Expected weight rows");
    KASSERT(weights_rows > 0, "Invalid weights # rows");

    unsigned int weights_cols = 0;
    KASSERT(ReadUnsignedInt(file, &weights_cols), "Expected weight cols");
    KASSERT(weights_cols > 0, "Invalid weights shape");

    unsigned int biases_shape = 0;
    KASSERT(ReadUnsignedInt(file, &biases_shape), "Expected biases shape");
    KASSERT(biases_shape > 0, "Invalid biases shape");

    weights_.Resize(weights_rows, weights_cols);
    KASSERT(
        ReadFloats(file, weights_.data_.data(), weights_rows * weights_cols),
        "Expected weights");

    biases_.Resize(biases_shape);
    KASSERT(ReadFloats(file, biases_.data_.data(), biases_shape),
            "Expected biases");

    KASSERT(activation_.LoadLayer(file), "Failed to load activation");

    return true;
}
std::vector<KerasLayer*> KerasModel::getLayers() const{
    return layers_;
}

Tensor KerasLayerDense::getWeights() const{
    return weights_;
}

Tensor KerasLayerDense::getBiases() const{
    return biases_;
}

Tensor KerasLayerActivation::getWeights() const{
    throw std::logic_error("Not Implemented");
    //return Tensor(); //Dummy
}

Tensor KerasLayerActivation::getBiases() const{
    throw std::logic_error("Not Implemented");
    //return Tensor(); //Dummy
}

bool KerasLayerDense::Apply(Tensor* in, Tensor* out) {
    Tensor tmp(weights_.dims_[1]);

    for (int i = 0; i < weights_.dims_[0]; i++) {
        for (int j = 0; j < weights_.dims_[1]; j++) {
            tmp(j) += (*in)(i)*weights_(i, j);
        }
    }

    for (int i = 0; i < biases_.dims_[0]; i++) {
        tmp(i) += biases_(i);
    }
    activation_.Apply(&tmp, out);

    return true;
}

bool KerasModel::LoadModel(const std::string& filename) {
    std::ifstream file(filename.c_str(), std::ios::binary);
    KASSERT(file.is_open(), "Unable to open file %s", filename.c_str());

    unsigned int num_layers = 0;
    KASSERT(ReadUnsignedInt(&file, &num_layers), "Expected number of layers");

    for (unsigned int i = 0; i < num_layers; i++) {
        unsigned int layer_type = 0;
        KASSERT(ReadUnsignedInt(&file, &layer_type), "Expected layer type");

        KerasLayer* layer = NULL;

        switch (layer_type) {
        case kDense:
            layer = new KerasLayerDense();
            break;
        default:
            break;
        }

        KASSERT(layer, "Unknown layer type %d", layer_type);

        bool result = layer->LoadLayer(&file);
        if (!result) {
            printf("Failed to load layer %d", i);
            delete layer;
            return false;
        }

        layers_.push_back(layer);
    }

    return true;
}

bool KerasModel::Apply(Tensor* in, Tensor* out) {
    Tensor temp_in, temp_out;

    for (unsigned int i = 0; i < layers_.size(); i++) {
        if (i == 0) {
            temp_in = *in;
        }
        layers_[i]->Apply(&temp_in, &temp_out);

        temp_in = temp_out;
    }

    *out = temp_out;

    return true;
}
