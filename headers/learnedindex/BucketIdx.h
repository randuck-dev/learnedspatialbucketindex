#pragma once

#include "Bucket.h"
#include "query.h"
#include <iostream>
#include <iomanip>
#include <string>
#include <limits>
#include <chrono>
#include <fstream>

using namespace std::chrono;
using namespace std;

struct NNDist {
	std::pair<double, double> nn;
	double dist;
};

bool flipped = false;

template <typename T>
class BucketIndex {
private:
	unsigned int m_dimensions;
	Bucket<T>* m_rootBucket;
	std::vector<NNDist> nnSet;

	NNDist nearestNeighborQuery_2d_one(Query_2d<T>& query, const std::pair<double, double>& nnq, double& epsilon, double& bound, int neighbors);
	std::vector<NNDist> nearestNeighborQuery_2d_many(Query_2d<T>& query, const std::pair<double, double>& nnq, double& epsilon, double& bound, int neighbors);

public:
	std::vector<tuple<int, std::chrono::nanoseconds::rep, int, std::chrono::nanoseconds::rep>> corrRate;
	std::vector<long long> totals;
	int occur = 0;
	std::vector<int> loops;
	std::vector<uint32_t> candidates;
	BucketIndex(const unsigned int& dimensions) : m_dimensions(dimensions) {};
	void setRootBucket(Bucket<T>* rootBucket);
	QueryEngineResult<T, T, T> rangeQuery(const Query<T, T, T>& query);
	void rangeQuery_5d(const Query_5d<T>& query);
	std::vector<NNDist> nearestNeighborQuery_2d(Query_2d<T>& query, const std::pair<double, double>& nnq, double& epsilon, double& bound, int neighbors);
	std::vector<std::pair<double, double>>* range_query2d(const Query_2d<T>& query);
	void insert(const Bucket<T>& inBucket);
};

template <typename T>
void BucketIndex<T>::setRootBucket(Bucket<T>* rootBucket) {
	if (rootBucket->m_children.size() < 1)
		std::cout << "WARNING: Bucket has no children" << std::endl;
	if (rootBucket->m_data.size() < 1)
		std::cout << "WARNING: Bucket has no data" << std::endl;

	m_rootBucket = rootBucket;
}

template <typename T>
QueryEngineResult<T, T, T> BucketIndex<T>::rangeQuery(const Query<T, T, T>& query) {

	QueryEngineResult<T, T, T> queryResult;
	int resultsize = 0;
	high_resolution_clock::time_point t_start = high_resolution_clock::now();
	int topLevelPredection = m_rootBucket.m_model.infer(query.dim_1_min);
	unsigned int min_idx = topLevelPredection;

	//Correct potential wrong top-level predections
	if (m_rootBucket.m_data[min_idx] != query.dim_1_min)
	{
		if (m_rootBucket.m_data[min_idx] > query.dim_1_min) {
			while (m_rootBucket.m_data[min_idx] != query.dim_1_min)
				min_idx--;
		}
		else {
			while (m_rootBucket.m_data[min_idx] != query.dim_1_min)
				min_idx++;
		}
	}

	while (m_rootBucket.m_data[min_idx] <= query.dim_1_max) {
		Bucket currentBucket = m_rootBucket.m_children[min_idx];
		int secondLevelPredection = currentBucket.m_model.infer(query.dim_2_min);
		//Correct potential wrong predections
		if (currentBucket.m_data[secondLevelPredection] != query.dim_2_min)
		{
			if (currentBucket.m_data[secondLevelPredection] > query.dim_2_min)
			{
				while (currentBucket.m_data[secondLevelPredection] > query.dim_2_min)
					secondLevelPredection--;
			}
			else {
				while (currentBucket.m_data[secondLevelPredection] < query.dim_2_min)
					secondLevelPredection++;
			}
		}
		while (currentBucket.m_data[secondLevelPredection] <= query.dim_2_max && secondLevelPredection < currentBucket.m_data.size()) {
			for (auto& discount : currentBucket.m_children[secondLevelPredection].m_data) {
				if (discount >= query.dim_3_min && discount <= query.dim_3_max) {
					// queryResult.resultVector.push_back(make_tuple(m_rootBucket.m_data[min_idx],
					//                                                currentBucket.m_data[secondLevelPredection],
					//                                                discount));
					resultsize++;
				}
			}
			secondLevelPredection++;
		}
		min_idx++;
	}
	high_resolution_clock::time_point t_end = high_resolution_clock::now();
	std::chrono::nanoseconds::rep total = duration_cast<nanoseconds>(t_end - t_start).count();
	queryResult.executionTime = total;
	std::cout << "size : " << resultsize << std::endl;
	return queryResult;
}

template <typename T>
void correctPredection(const std::vector<T> & data, uint32_t & idx, const T & target) {

 	if (target < data[0]) {
		flipped = true;
		idx = 0;
		return;
	}
	if (target > data[data.size() - 1]) {
		flipped = true;
		idx = data.size() - 1;
		return;
	}
	if (idx > data.size() - 1) {
		idx = data.size() - 1;
	}
	if (idx < 0) {
		idx = 0;
	}

	if (data[idx] == target) {
		return; //Do check in query func to avoid call?
	}
	if (data[idx] > target) {
		while (data[idx] > target) {
			idx--;
		}
		if (data[idx] != target)
			idx++;
	}
	else {
		while (data[idx] < target) {
			idx++;
		}
		// if(data[idx] != target)
		//     idx--;
	}
}

template<typename T>
std::pair<bool, int> findInVector(const std::vector<T> & vecOfElements, const T & element) {
	std::pair<bool, int> result;
	// Find given element in vector
	auto it = std::find(vecOfElements.begin(), vecOfElements.end(), element);

	if (it != vecOfElements.end())
	{
		result.second = distance(vecOfElements.begin(), it);
		result.first = true;
	}
	else
	{
		result.first = false;
		result.second = -1;
	}

	return result;
}

void printQueryResult(vector<tuple<uint32_t, uint32_t, uint32_t, uint32_t, uint32_t>> & results, const chrono::nanoseconds::rep & executionTime) {
	const int quantity_width = 10;
	const int discount_width = 12;
	const int date_width = 15;
	const string sep = " |";
	const int total_width = quantity_width * 2 + discount_width + date_width * 2 + sep.size() * 5;
	const string line = sep + string(total_width - 1, '-') + '|';
	const int executionTimeWidth = date_width;
	const int rowsWidth = discount_width + quantity_width + size(to_string(results.size()));


	cout << line << "\n"
		<< sep << setw(quantity_width) << "CUSTKEY"
		<< sep << setw(date_width) << "ORDERDATE"
		<< sep << setw(date_width) << "SHIPDATE"
		<< sep << setw(quantity_width) << "QUANTITY"
		<< sep << setw(discount_width) << "DISCOUNT"
		<< sep << "\n" << line << "\n";

	// cout << line << "\n" << sep
	//      << setw(quantity_width) << "Quantity" << sep << setw(discount_width) << "Discount" << sep << setw(date_width) << "Date" << sep << "\n" << line << "\n";

	// // //Hack for output to avoid newline in ctime return value (hence results not marked as 'const')
	// // for(auto& result : results){
	// //     int end = get<2>(result).find('\n');
	// //     get<2>(result) = get<2>(result).substr(0,end);
	// // }


	for (auto& result : results) {
		cout << sep << setw(quantity_width) << get<0>(result)
			<< sep << setw(date_width) << get<1>(result)
			<< sep << setw(date_width) << get<2>(result)
			<< sep << setw(quantity_width) << get<3>(result)
			<< sep << setw(discount_width) << get<4>(result)
			<< sep << "\n";
	}
	cout << line << "\n";
	//Fyyyy hardcoded much...
	string r = "Rows: " + to_string(results.size());
	string e = "Execution time: " + to_string(executionTime) + " ns";
	cout << sep << setw(quantity_width + date_width + 2) << r
		<< sep << setw(date_width + quantity_width + discount_width + 4) << e << sep << "\n";
	// cout << sep << setw(rowsWidth) << "Rows: " << results.size() << sep << setw(executionTimeWidth) <<
	//     "Execution time: " << executionTime << " ns" << sep << "\n";
	cout << line << "\n";
}


template <typename T>
void BucketIndex<T>::rangeQuery_5d(const Query_5d<T> & query) {
	int resultsize = 0;
	int model_usage = 0;
	vector<tuple<uint32_t, uint32_t, uint32_t, uint32_t, uint32_t>> results;
	high_resolution_clock::time_point t_start = high_resolution_clock::now();

	uint32_t topPred = m_rootBucket->m_model.infer(query.dim_1_min);
	//std::cout << topPred << std::endl;
	correctPredection(m_rootBucket->m_data, topPred, query.dim_1_min);

	while (m_rootBucket->m_data[topPred] <= query.dim_1_max && topPred < m_rootBucket->m_data.size()) {
		Bucket<T>* secBucket = &m_rootBucket->m_children[topPred];
		uint32_t secPred;
		if (secBucket->m_model.isModelSet) {
			secPred = secBucket->m_model.infer(query.dim_2_min);
			model_usage++;
			correctPredection(secBucket->m_data, secPred, query.dim_2_min);
		}
		else {
			//std::cout << "MODEL2 NOT SET!!" << std::endl;
			correctPredection(secBucket->m_data, secPred, query.dim_2_min);
		}

		while (secBucket->m_data[secPred] <= query.dim_2_max && secPred < secBucket->m_data.size()) {
			Bucket<T>* thiBucket = &secBucket->m_children[secPred];
			uint32_t thiPred;
			if (thiBucket->m_model.isModelSet) {
				model_usage++;
				thiPred = thiBucket->m_model.infer(query.dim_3_min);
				correctPredection(thiBucket->m_data, thiPred, query.dim_3_min);
			}
			else {
				//std::cout << "MODEL3 NOT SET!!"  << std::endl;
				correctPredection(thiBucket->m_data, thiPred, query.dim_3_min);
			}

			while (thiBucket->m_data[thiPred] <= query.dim_3_max && thiPred < thiBucket->m_data.size()) {
				Bucket<T>* fouBucket = &thiBucket->m_children[thiPred];
				uint32_t fouPred;
				if (fouBucket->m_model.isModelSet) {
					model_usage++;
					fouPred = fouBucket->m_model.infer(query.dim_4_min);
					correctPredection(fouBucket->m_data, fouPred, query.dim_4_min);
				}
				else {
					//std::cout << "MODEL4 NOT SET!!" << std::endl;
					correctPredection(fouBucket->m_data, fouPred, query.dim_4_min);
				}
				while (fouBucket->m_data[fouPred] <= query.dim_4_max && fouPred < fouBucket->m_data.size()) {
					for (auto& entry : fouBucket->m_children[fouPred].m_data) {
						if (entry >= query.dim_4_min && entry <= query.dim_5_max) {
							//Expensive to do? Should we do this?
							// results.push_back(std::make_tuple(m_rootBucket->m_data[topPred],
							//                                   secBucket->m_data[secPred],
							//                                   thiBucket->m_data[thiPred],
							//                                   fouBucket->m_data[fouPred],
							//                                   entry));
							resultsize++;
						}
					}
					fouPred++;
				}
				thiPred++;
			}
			secPred++;
		}
		topPred++;
	}

	high_resolution_clock::time_point t_end = high_resolution_clock::now();
	std::chrono::nanoseconds::rep t_total = duration_cast<nanoseconds>(t_end - t_start).count();
	if (!results.empty())
		printQueryResult(results, t_total);
	std::cout << "EXECUTION TIME: " << t_total << std::endl;
	std::cout << "RESULT SIZE: " << resultsize << std::endl;
	std::cout << "MODEL USAGE: " << model_usage << std::endl;

}

enum class Direction {
	UP, DOWN, NONE
};

bool topFlipped = true;

template<typename T>
std::vector<Bucket<T>*>& correctPredictionBasedOnBounds(uint32_t prediction, Bucket<T> * parentBucket, const Query_2d<T> & query, const std::pair<float, float> & nnq, std::vector<Bucket<T>*> & candidateset, const Direction & dir, const float& bound, const uint32_t & maxElements) {
	if (prediction <= 0 && (dir == Direction::DOWN)) { // Did we come from a previous recursive call that is going towards zero
		prediction = 0;
		Bucket<T>* candidate = &(parentBucket->m_children[prediction]);
		candidateset.push_back(candidate);
		return candidateset;
	}
	if (prediction >= maxElements - 1 && (dir == Direction::UP)) { // Did we predict over the range and are we already moving in this direction
		prediction = maxElements - 1;
		Bucket<T>* candidate = &(parentBucket->m_children[prediction]);
		candidateset.push_back(candidate);
		return candidateset;
	}
	if (prediction >= maxElements - 1) {
		prediction = maxElements - 1;
	}
	Bucket<T>* candidate = &(parentBucket->m_children[prediction]);

	if (candidate->contains(nnq.first, bound)) { // Add if we have a bucket that contains the item
		candidateset.push_back(candidate);
	}

	if (nnq.first - bound < candidate->lower && (dir != Direction::UP)) { // Go down if a query is smaller than the smaller range
		prediction--;
		correctPredictionBasedOnBounds(prediction, parentBucket, query, nnq, candidateset, Direction::DOWN, bound, maxElements);
	}

	if (nnq.first + bound > candidate->upper && (dir != Direction::DOWN)) { // Go up if a query is larger than the upper range
		prediction++;
		correctPredictionBasedOnBounds(prediction, parentBucket, query, nnq, candidateset, Direction::UP, bound, maxElements);
	}
	return candidateset;
}

template<typename T>
void correctPredictionRangeBound(uint32_t & prediction, const Bucket<T> & root, const T & target) {
	if (target < root.m_children[0].lower) { // Must be from here that we start to invesigate
		topFlipped = true;
		prediction = 0;
		return;
	}
	if (target > root.m_children[root.m_children.size() - 1].upper) { // No candidates
		topFlipped = true;
		prediction = root.m_children.size() - 1;
		return;
	}
	if (prediction > root.m_children.size() - 1) {
		prediction = root.m_children.size() - 1;
	}

	if (prediction < 0) {
		prediction = 0;
	}
	if (root.m_children[prediction].lower <= target && root.m_children[prediction].upper >= target) {// The target can be within smaller and upper
		return;
	}
	if (root.m_children[prediction].upper < target && prediction == root.m_children.size() - 1) {
		while (root.m_children[prediction].upper < target && prediction == root.m_children.size() - 1) { // The target can be larger than both the lower and the upper
			prediction++;
			if (prediction == root.m_children.size() - 1) {
				return;
			}
		}
	}
	else {
		while (root.m_children[prediction].upper > target && prediction > 0) { // The target can be smaller than both the lower and the upper
			prediction--;
			if (prediction == 0) {
				return;
			}
		}
	}
}

template<typename T>
std::vector<std::pair<double, double>>* BucketIndex<T>::range_query2d(const Query_2d<T> & query)
{
	auto* pairs = new std::vector<std::pair<double, double>>();

	uint32_t topPred = m_rootBucket->m_model->infer(query.dim_1_min);
	uint32_t predTopIdx = topPred;
	
	correctPredictionRangeBound(topPred, *m_rootBucket, query.dim_1_min);
	bool isLooping = true;
	while (isLooping && topPred < m_rootBucket->m_children.size()) {
		Bucket<T>* candidate = &(m_rootBucket->m_children[topPred]);
		if ((candidate->children_lower >= query.dim_2_min && candidate->children_upper <= query.dim_2_max) || (query.dim_2_min >= candidate->children_lower && query.dim_2_min <= candidate->children_upper) || (query.dim_2_max >= candidate->children_lower && query.dim_2_max <= candidate->children_upper)) {
			if (candidate->upper > query.dim_1_max) { // This is the last bucket we investigate, but there can still be something in the range starting from lower
				isLooping = false;
			}

			uint32_t pred = candidate->m_model->infer(query.dim_1_min);
			correctPredection<T>(candidate->m_data, pred, query.dim_1_min);
			
			if (candidate->m_data[pred] < query.dim_1_min) {	
				pred++;
			}
			while (candidate->m_data[pred] <= query.dim_1_max && pred < candidate->m_data.size()) {
				const auto& x = candidate->m_children[pred];

				if ((x.lower >= query.dim_2_min && x.upper <= query.dim_2_max) || (query.dim_2_min >= x.lower && query.dim_2_min <= x.upper) || (query.dim_2_max >= x.lower && query.dim_2_max <= x.upper)) {
					for (auto& entry : x.m_data) {
						if (entry > query.dim_2_max) {
							break;
						}
						if (query.dim_2_min <= entry && entry <= query.dim_2_max) {
							pairs->push_back(std::make_pair(x.m_id, entry));
						}
					}
				}
				pred++;
			}
		}	
		topPred++;
	}

	return pairs;
}

std::vector<double> bounds;
std::vector<double> sizes;

template<typename T>
NNDist BucketIndex<T>::nearestNeighborQuery_2d_one(Query_2d<T>& query, const std::pair<double, double>& nnq, double& epsilon, double& bound, int neighbors) {
	std::pair<double, double> nearestNeighbor;

	int looping = 0;
	int i = 0;
	int threshold = 2;

	std::vector<std::pair<double, double>>* nnCandidates = range_query2d(query);

	while (nnCandidates->size() == 0) { // Make the range query larger, when there is nothing in the boundary
		bound = bound + (bound * epsilon);
		bound = sqrt(pow(bound, 2) + pow(bound, 2));
		nnCandidates = range_query2d({ nnq.first - bound, nnq.first + bound, nnq.second - bound, nnq.second + bound });

		looping++;
		i++;
	}
	loops.push_back(looping);
	candidates.push_back(nnCandidates->size());
	double minDist = (std::numeric_limits<double>::max)();
	bool isInit = false;

	for (const auto& candidate : *nnCandidates) { // TODO I think we can do this faster. Create a few threads and then go at it
		double dist{ utils::euclideanDistance(nnq, candidate) };
		if (dist < minDist && dist < bound) { // There must be something within the radius that is a viable candidate
			isInit = true;
			nearestNeighbor = candidate;
			minDist = dist;
		}
	}
	if (!isInit) {
		bound = bound + (bound * (epsilon / 2));
		bound = sqrt(pow(bound, 2) + pow(bound, 2));
		Query_2d<T> newQ = {
			nnq.first - bound, nnq.first + bound,
			nnq.second - bound, nnq.second + bound
		};
		return nearestNeighborQuery_2d_one(newQ, nnq, epsilon, bound, neighbors);
	}

	delete nnCandidates;
	return { nearestNeighbor, minDist };
}

template<typename T>
std::vector<NNDist> BucketIndex<T>::nearestNeighborQuery_2d_many(Query_2d<T>& query, const std::pair<double, double>& nnq, double& epsilon, double& bound, int neighbors) {
	std::pair<double, double> nearestNeighbor;

	int looping = 0;
	int i = 0;
	int threshold = 2;

	std::vector<std::pair<double, double>>* nnCandidates = range_query2d(query);

	while (nnCandidates->size() == 0) { // Make the range query larger, when there is nothing in the boundary
		bound = bound + (bound * epsilon);
		bound = sqrt(pow(bound, 2) + pow(bound, 2));
		nnCandidates = range_query2d({ nnq.first - bound, nnq.first + bound, nnq.second - bound, nnq.second + bound });

		looping++;
		i++;
	}
	loops.push_back(looping);
	candidates.push_back(nnCandidates->size());
	double minDist = (std::numeric_limits<double>::max)();
	bool isInit = false;

	for (const auto& candidate : *nnCandidates) { // TODO I think we can do this faster. Create a few threads and then go at it
		double dist{ utils::euclideanDistance(nnq, candidate) };
		if (dist < minDist && dist < bound) { // There must be something within the radius that is a viable candidate
			isInit = true;
			nearestNeighbor = candidate;
			minDist = dist;
			nnSet.push_back(NNDist{ nearestNeighbor, minDist });
		}
	}
	if (nnSet.size() < neighbors || !isInit) {
		bound = bound + (bound * (epsilon / 2));
		bound = sqrt(pow(bound, 2) + pow(bound, 2));
		Query_2d<T> newQ = {
			nnq.first - bound, nnq.first + bound,
			nnq.second - bound, nnq.second + bound
		};
		return nearestNeighborQuery_2d_many(newQ, nnq, epsilon, bound, neighbors);
	}

	delete nnCandidates;
	std::sort(nnSet.begin(), nnSet.end(), [](const auto & a, const auto & b) {return a.dist < b.dist; });

	return std::vector<NNDist>{nnSet.begin(), nnSet.begin() + neighbors};
}


template<typename T>
std::vector<NNDist> BucketIndex<T>::nearestNeighborQuery_2d(Query_2d<T> & query, const std::pair<double, double>& nnq, double& epsilon, double& bound, int neighbors) {
	if (neighbors == 1) {
		std::vector<NNDist> val;
		val.push_back(nearestNeighborQuery_2d_one(query, nnq, epsilon, bound, neighbors));
		return val;
	}
	else {
		return nearestNeighborQuery_2d_many(query, nnq, epsilon, bound, neighbors);
	}
}


//-----INSERTION-----//
template <typename T>
std::pair<bool, uint32_t> keyExist(const std::vector<T> & data, const T & target, uint32_t predIdx) {
	if (data[predIdx] == target) {
		return make_pair(true, predIdx);
	}
	if (data[predIdx] > target) {
		while (data[predIdx] > target) {
			if (data[predIdx] == target)
				return make_pair(true, predIdx);
			predIdx--;
		}
		return make_pair(false, predIdx + 1);
	}
	while (data[predIdx] < target) {
		if (data[predIdx] == target)
			return make_pair(true, predIdx);
		predIdx++;
	}
	return make_pair(false, predIdx - 1);
}

template <typename T>
void BucketIndex<T>::insert(const Bucket<T> & inBucket) {

	double prediction = m_rootBucket->m_model.infer(inBucket.m_data[0]);
	auto [found, idx] = keyExist(m_rootBucket->m_data, inBucket.m_data[0], int(round(prediction)));

}


