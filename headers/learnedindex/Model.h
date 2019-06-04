
#pragma once

#include "keras_model.h"

class IModel {
public:

	bool isModelSet{};
	virtual ~IModel() {}
	virtual float infer(const float& input) = 0;
};

class NNModel : public IModel {
public:
	KerasModel kerasModel;
	float infer(const float& input) override {
		Tensor in(1);
		Tensor out(1);
		in.data_ = {
			{input}
		};
		kerasModel.Apply(&in, &out);

		return out.data_[0];
	}
};

