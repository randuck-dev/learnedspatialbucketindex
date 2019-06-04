//
// Created by Raphael Neumann on 2019-04-08.
//

#pragma once

#include <vector>
#include <algorithm>
#include "Bucket.h"
#include "extlib/groupby.h"
#include "lg.h"
#include "LinearModel.h"
#include "Model.h"
#include <random>
#include "utils.h"
#include <assert.h>

#ifdef DEBUG
#  define D(x) std::cerr << x << std::endl;
#else
#  define D(x)
#endif // DEBUG

#define I(x) std::cout << x << std::endl;

bool double_equals(const double a, const double b, const double epsilon = 0.001) {
    return std::abs(a - b) < epsilon;
}

LinearModel* train(const std::vector<double> &data, const std::vector<double> &predictors) {
	assert(data.size() == predictors.size());
    LinearModel* lg = new LinearModel();

    linear_regression(data, predictors, data.size(), *lg);
	if (isnan(lg->bias) || isnan(lg->weight)) {
		
		//throw std::exception("bias or weight are NAN");
		throw std::invalid_argument("Bias or weight are NAN");
	}
    return lg;
}

class DataStore {
public:
	std::vector<std::vector<float>> getData;
};

template<typename T>
class BucketCreatorV2 {
public:

	uint32_t i = 0;
	uint32_t successful = 0;
	uint32_t noLinearModels = 0;
	uint32_t createdBuckets = 0;
	uint32_t lastDimCounter = 0;
	std::string modelName;
	uint32_t bucketsize;
	Bucket<T> bucket(std::vector<std::vector<T>>& rows, const uint32_t currDimension, const uint32_t totalDimensions,
		const std::vector<std::string>& dimensionNames) {

		performBucketing(rows);
		IModel* model = getModel();
		topBucky.m_id = 1;
		model->isModelSet = true;
		topBucky.m_model = model;
		createdBuckets++;
		topBucky.layer = currDimension;
		
		bucketInternal(rows, currDimension, totalDimensions, dimensionNames, topBucky, 0);
		return topBucky;

	}

protected:
	Bucket<T> topBucky;

private:
	
	virtual void performBucketing(std::vector<std::vector<T>>& rows) = 0;
	virtual IModel* getModel() = 0;
	virtual void bucketInternal(std::vector<std::vector<T>>& rows, const uint32_t& currDimension, const uint32_t& totalDimensions,
		const std::vector<std::string>& dimensionNames, Bucket<T>& parent, const uint32_t recDepth) = 0;
};

template<typename T>
class SpatialBucketCreator : public BucketCreatorV2<T>{};

template<>
class SpatialBucketCreator<double> : public BucketCreatorV2<double> {
private:
	NNModel* model;
	void performBucketing(std::vector<std::vector<double>>& rows) override {
		int BUCKETSIZE = this->bucketsize;

		int remainder = rows.size() % BUCKETSIZE;
		int n_iterations = (rows.size() - remainder) / BUCKETSIZE;
		std::cout << "[BUCKETIDX] Remainder: " << remainder << std::endl;
		std::sort(rows.begin(), rows.end(), [](const auto & a, const auto & b) { return a[0] < b[0]; });

		std::vector<double> topLevelData;
		for (uint32_t i = 0; i < rows.size(); i++) {
			topLevelData.push_back(rows[i][0]);
		}
		
		topBucky.m_data = topLevelData;

		long long counter = 0;
		uint32_t bucketId = 0;
		std::cout << "[BUCKETIDX] DATASIZE: " << rows.size() << std::endl;
		auto bucketize = [&](const int N_ITER) {
			std::vector<double> vals;
			double low = rows[counter][0];
			
			for (int j = 0; j < N_ITER; j++) {
				vals.push_back(rows[counter][0]); // 0 is first dimension
				counter++;
			}
			double high = vals[vals.size() - 1];
			Bucket<double> bucket;
			
			std::sort(vals.begin(), vals.end(),
				[](const double& a, const double& b) { return a < b; }); // Have to sort, so that unique works
			
			std::vector<double> uniqueValues;

			vals.erase(std::unique(vals.begin(), vals.end()), vals.end());

			std::vector<double> positions;
			for (uint32_t pos = 0; pos < vals.size(); pos++) {
				positions.push_back(pos);
				uniqueValues.push_back(vals[pos]);
			}
			LinearModel* model;
			if (uniqueValues.size() > 2) {
				 model = train(uniqueValues, positions);
				 model->isModelSet = true;
			}
			else {
				model = new LinearModel();
				model->isModelSet = false;
			}
			bucket.m_data = vals;
			bucket.m_model = model;
			bucket.lower = low;
			bucket.upper = high;
			topBucky.m_children.push_back(bucket);
		};
		for (int i = 0; i < n_iterations; i++) {
			bucketize(BUCKETSIZE);
		}
		if (remainder > 0) {
			bucketize(remainder);
		}
		std::cout << "[BUCKETIDX] COUNTER: " << counter << std::endl;
		
		auto xtoy = utils::getXMappedToY(rows);
		long long y_counter = 0;
		
		for (Bucket<double>& buck : topBucky.m_children) { 
			auto unique = buck.m_data;
			unique.erase(std::unique(unique.begin(), unique.end()), unique.end());

			std::vector<double> y_lowers;
			std::vector<double> y_uppers;

			for (double& val : unique) {
				Bucket<double> y_bucket;
				auto& yvals = xtoy[val];

				y_counter += yvals.size();
				
				std::sort(yvals.begin(), yvals.end(), [](const auto & a, const auto & b) { return a < b; });
	
				y_bucket.m_id = val;
				y_bucket.lower = yvals[0];
				y_bucket.upper = yvals[yvals.size() - 1];
				y_bucket.m_data = yvals;
				y_bucket.m_model = new NNModel();
				y_bucket.m_model->isModelSet = false;
				y_uppers.push_back(yvals[yvals.size() - 1]);
				y_lowers.push_back(yvals[0]);
				buck.m_children.push_back(y_bucket);
			}

			std::sort(y_uppers.begin(), y_uppers.end());
			std::sort(y_lowers.begin(), y_lowers.end());

			auto y_lowest = y_lowers[0];
			auto y_largest = y_uppers[y_uppers.size() - 1];
			buck.children_lower = y_lowest;
			buck.children_upper = y_largest;
		}
		KerasModel n_model = utils::load_model(this->modelName);

		std::cout << "[BUCKETIDX] TOTALLY INSERTED: " << counter << std::endl;
		std::cout << "[BUCKETIDX] TOTALLY INSERTED Y: " << y_counter << std::endl; 
		std::cout << "[BUCKETIDX] TOTALLY INSERTED DIFF: " << abs(y_counter-counter) << std::endl;
		model = new NNModel();
		model->kerasModel = n_model;
	}

	virtual IModel* getModel() override {
		return model;
	}

	virtual void bucketInternal(std::vector<std::vector<double>>& rows, const uint32_t& currDimension, const uint32_t& totalDimensions,
		const std::vector<std::string>& dimensionNames, Bucket<double>& parent, const uint32_t recDepth) override {
		//std::cout << "CALLED INTERNAL POI BUCKETING" << std::endl;
	}
};


template<typename T>
class TpcHBucketCreator : public BucketCreatorV2<T> {
private:
	LinearModel* lm;
	virtual void performBucketing(std::vector<std::vector<T>>& rows) override {
		std::vector<T> firstDimData;
		std::vector<double> positions;
		uint32_t i = 0;
		for (auto&& item : iter::groupby(rows, [](std::vector<T>& it) { return it[0]; })) {
			firstDimData.push_back(item.first);
			positions.push_back(i);
			i++;
		}

		std::vector<double> doubled(firstDimData.begin(), firstDimData.end());
		lm = train(doubled, positions);
		this->topBucky.m_data = firstDimData;
		//topBucky.m_data = firstDimData;
	}


	virtual void bucketInternal(std::vector<std::vector<T>>& rows, const uint32_t& currDimension, const uint32_t& totalDimensions,
		const std::vector<std::string>& dimensionNames, Bucket<T>& parent, const uint32_t recDepth) override {
		const uint32_t nextDimensionVal = currDimension + 1;
		if (recDepth > 4) {
			std::cout << "REC-DEPTH: " << recDepth << std::endl;
		}

		if (nextDimensionVal >= totalDimensions || rows.size() == 0) // We do not group and linearly regress on the last dimension
		{
			D("");
			D("Returning on " << dimensionNames[currDimension]);
			this->lastDimCounter++;
			return;
		}

		D("Bucketing dimension: " << nextDimensionVal << " of " << totalDimensions - 1);
		D(">>> Grouping on Dimension: " << dimensionNames[currDimension])
			D(">>> Linear regression on Dimension: " << dimensionNames[nextDimensionVal]);
		std::sort(rows.begin(), rows.end(),
			[](const std::vector<T> & a, const std::vector<T> & b) { return a[0] < b[0]; });
		
		std::map<T, std::vector<std::vector<T>>> dimensionToRows;
		std::map<T, std::vector<T>> dimToNextDim;
		for (auto&& item : iter::groupby(rows, [](std::vector<T>& it) { return it[0]; })) {
			std::vector<std::vector<T>> nextRows;
			std::vector<T> nextDimension;
			for (const auto& val : item.second) {
				nextDimension.push_back(val[1]);
				int counter{ 0 };
				std::vector<T> ro(val.size() - 1);
				bool isPlaced = false;
				for (uint32_t idx = 0; idx < val.size(); idx++) {
					if (counter == 0) {
						counter++;
						continue;
					}
					else {
						isPlaced = true;
						counter++;
						ro[idx - 1] = val[idx];
					}
				}
				if (isPlaced) { // We have to make this check because it can happen that no items are placed in the vector to be pushed
					nextRows.emplace_back(std::move(ro));
				}
			}
			dimToNextDim.insert(std::make_pair(item.first, nextDimension));
			dimensionToRows.insert(std::make_pair(item.first, nextRows));
		}
		std::vector<std::vector<T>>().swap(rows);

		for (auto&& item : dimensionToRows) {

			auto nextDimension = dimToNextDim.find(item.first)->second;

			std::sort(nextDimension.begin(), nextDimension.end(),
				[](const double& a, const double& b) { return a < b; }); // Have to sort, so that unique works
			std::vector<double> uniqueValues;
			nextDimension.erase(std::unique(nextDimension.begin(), nextDimension.end()), nextDimension.end());

			std::vector<double> positions;
			for (uint32_t pos = 0; pos < nextDimension.size(); pos++) {
				positions.push_back(pos);
				uniqueValues.push_back(nextDimension[pos]);
			}

			D(">>> Size of NextRows: " << nextRows.size());
			D(">>> Size of Quantizied: " << uniqueValues.size());
			LinearModel * model = trainInternalModel(uniqueValues, positions);

			Bucket<T> bucky;
			bucky.lower =
				bucky.m_id = item.first;
			//if (dimensionNames[nextDimensionVal] == "DISCOUNT") // We do not group and linearly regress on the last dimension
			//{
			//	return;
			//}
			bucketInternal(item.second, nextDimensionVal, totalDimensions, dimensionNames, bucky, recDepth + 1);

			this->createdBuckets++;
			bucky.m_data = std::move(nextDimension);
			bucky.m_model = model;
			bucky.layer = currDimension + 1;
			parent.m_children.emplace_back(std::move(bucky));

		}

	}

	LinearModel* trainInternalModel(std::vector<double> values, std::vector<double> predictors) {
		LinearModel* model;
		const auto normsize = (uint32_t)values.size();
		if (normsize >= 3) // Train the linear regression model for each group in the dimension
		{
			model = train(values, predictors);
			model->isModelSet = true;

			this->i += 1;
			auto idx = 1;
			//auto idx = std::experimental::randint<uint32_t>(0, normsize - 1);
			double xval = values[idx];
			double actual = predictors[idx];
			double yval = model->infer(xval);

			if (yval > normsize) { // If we predict something above the range
				yval = normsize;
			}
			else if (yval < 0) { // If we predict something below the range
				yval = 0;
			}

			if (double_equals(yval, actual, 7) || ((int)yval) == ((int)actual)) {
				this->successful++;
			}
		}
		else // Linear regression results in NAN when training on less than 3 elements
		{
			model = new LinearModel();
			model->isModelSet = false;
			this->noLinearModels++;
		}
		return model;
	}

	virtual IModel* getModel() override {
		return lm;
	}
};


class BucketCreator {

public:
	uint32_t i = 0;
	uint32_t successful = 0;
	uint32_t noLinearModels = 0;
	uint32_t createdBuckets = 0;
	uint32_t lastDimCounter = 0;
    template<typename T>
    Bucket<T> bucket(std::vector<std::vector<T>>& rows, const uint32_t currDimension, const uint32_t totalDimensions,
                     const std::vector<std::string> &dimensionNames) {

		std::vector<T> firstDimData;
        std::vector<double> positions;
        uint32_t i = 0;
        for (auto &&item : iter::groupby(rows, [](std::vector<T> &it) { return it[0]; })) {
            firstDimData.push_back(item.first);
            positions.push_back(i);
            i++;
        }
        D(firstDimData.size());
        std::vector<double> doubled(firstDimData.begin(), firstDimData.end());
		
        IModel* model = train(doubled, positions);
        doubled.clear();
        positions.clear();
		Bucket<T> topBucky;
		topBucky.m_id = 1;
		model->isModelSet = true;
        topBucky.m_model = model;
		createdBuckets++;
		topBucky.layer = currDimension;
        
        topBucky.m_data = firstDimData;
        bucketInternal(rows, currDimension, totalDimensions, dimensionNames, topBucky, 0);
        return topBucky;
    }


private:
	template<typename T>
    void bucketInternal(std::vector<std::vector<T>>& rows, const uint32_t& currDimension, const uint32_t& totalDimensions,
                        const std::vector<std::string> &dimensionNames, Bucket<T> &parent, const uint32_t recDepth) {
        const uint32_t nextDimensionVal = currDimension + 1;
		if (recDepth > 4) {
			std::cout << "REC-DEPTH: " << recDepth << std::endl;
		}
	
        if (nextDimensionVal >= totalDimensions || rows.size() == 0) // We do not group and linearly regress on the last dimension
        {
            D("");
			D("Returning on " << dimensionNames[currDimension]);
			lastDimCounter++;
            return;
        }

        D("Bucketing dimension: " << nextDimensionVal << " of " << totalDimensions - 1);
        D(">>> Grouping on Dimension: " << dimensionNames[currDimension])
        D(">>> Linear regression on Dimension: " << dimensionNames[nextDimensionVal]);
        std::sort(rows.begin(), rows.end(),
                  [](const std::vector<T> &a, const std::vector<T> &b) { return a[0] < b[0]; });

		std::map<T, std::vector<std::vector<T>>> dimensionToRows;
		std::map<T, std::vector<T>> dimToNextDim;
		for (auto &&item : iter::groupby(rows, [](std::vector<T>& it) { return it[0]; })) {
			std::vector<std::vector<T>> nextRows;
			std::vector<T> nextDimension;
			for (const auto &val : item.second) {
				nextDimension.push_back(val[1]);
				int counter{ 0 };
				std::vector<T> ro(val.size() - 1);
				bool isPlaced = false;
				for (uint32_t idx = 0; idx < val.size(); idx++) {
					if (counter == 0) {
						counter++;
						continue;
					}
					else {
						isPlaced = true;
						counter++;
						ro[idx - 1] = val[idx];
					}
				}
				if (isPlaced) { // We have to make this check because it can happen that no items are placed in the vector to be pushed
					nextRows.emplace_back(std::move(ro));
				}
			}
			dimToNextDim.insert(std::make_pair(item.first, nextDimension));
			dimensionToRows.insert(std::make_pair(item.first, nextRows));
		}
		std::vector<std::vector<T>>().swap(rows);
	
		for(auto &&item: dimensionToRows) {
            
			auto nextDimension = dimToNextDim.find(item.first)->second;
			
            std::sort(nextDimension.begin(), nextDimension.end(),
                      [](const double &a, const double &b) { return a < b; }); // Have to sort, so that unique works
            std::vector<double> uniqueValues;
            nextDimension.erase(std::unique(nextDimension.begin(), nextDimension.end()), nextDimension.end());

            std::vector<double> positions;
            for (uint32_t pos = 0; pos < nextDimension.size(); pos++) {
                positions.push_back(pos);
                uniqueValues.push_back(nextDimension[pos]);
            }

            D(">>> Size of NextRows: " << nextRows.size());
            D(">>> Size of Quantizied: " << uniqueValues.size());
            const auto normsize = (uint32_t) uniqueValues.size();
			LinearModel* model;
			
            if (normsize >= 3) // Train the linear regression model for each group in the dimension
            {
                model = train(uniqueValues, positions);
                model->isModelSet = true;

                i += 1;
				auto idx = 1;
                //auto idx = std::experimental::randint<uint32_t>(0, normsize - 1);
                double xval = uniqueValues[idx];
                double actual = positions[idx];
                double yval = model->infer(xval);

                if (yval > normsize) { // If we predict something above the range
                    yval = normsize;
                } else if (yval < 0) { // If we predict something below the range
                    yval = 0;
                }


                if (double_equals(yval, actual, 7) || ((int) yval) == ((int) actual)) {
                    successful++;
                }
            } else // Linear regression results in NAN when training on less than 3 elements
            {
				model = new LinearModel();
				model->isModelSet = false;
                noLinearModels++;
            }
			Bucket<T> bucky;
			bucky.lower = 
            bucky.m_id = item.first;
			//if (dimensionNames[nextDimensionVal] == "DISCOUNT") // We do not group and linearly regress on the last dimension
			//{
			//	return;
			//}
			bucketInternal(std::move(item.second), nextDimensionVal, totalDimensions, dimensionNames, bucky, recDepth+1);

			createdBuckets++;
			bucky.m_data = std::move(nextDimension);
			bucky.m_model = model;
			bucky.layer = currDimension+1;
            parent.m_children.emplace_back(std::move(bucky));
			
        }
	
    }
};


