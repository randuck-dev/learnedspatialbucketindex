#ifndef IBS_H
#define IBS_H

#include <vector>
#include <numeric>
#include "GeoPoint.h"
#include "utils.h"

//TODO regarding all strategies...what to do if hash area only contains a single point IMPORTANT! (currently discards single point regions)
class IBoundEsimationStrategy {
public:
    virtual double calcBound(const std::vector<GeoPoint>& data) = 0;
};

class WorstCaseEstimator : public IBoundEsimationStrategy{
    public:
        double calcBound(const std::vector<GeoPoint>& data) override {
            double worst = -1;
            for (size_t i = 0; i < data.size() - 1; i++)
            {
                //double dist = utils::euclideanDistance(data[i], data[i+1]);
                double dist = abs(data[i].lat - data[i+1].lat);
                if(dist > worst)
                    worst = dist;
            }
            return worst;
        }
};

class BestCaseEstimator : public IBoundEsimationStrategy{
    public:
        double calcBound(const std::vector<GeoPoint>& data) override {
            double best = (std::numeric_limits<double>::max)();
            for (size_t i = 0; i < data.size() - 1; i++)
            {
                // double dist = utils::euclideanDistance(data[i], data[i+1]);
                // if(dist < best)
                //     best = dist;
                double dist = abs(data[i].lat - data[i+1].lat);
                if(dist < best)
                    best = dist;
            }
            return best;
        }
};

class AverageCaseOnePassEstimator : public IBoundEsimationStrategy {
public:
	double calcBound(const std::vector<GeoPoint>& data) override {
		std::vector<double> distances;
		for (size_t i = 0; i < data.size() - 1; i++)
		{
			distances.push_back(abs(data[i].lat - data[i + 1].lat));
		}
		return (std::accumulate(distances.begin(), distances.end(), 0.0)) / data.size();
	}
};

class MedianCaseEstimator : public IBoundEsimationStrategy {
    public:
        double calcBound(const std::vector<GeoPoint>& data) override {
            std::vector<double> dists;
            for (size_t i = 0; i < data.size() - 1; i++)
            {
                double dist = utils::euclideanDistance(data[i], data[i + 1]);
                dists.push_back(dist);
            
            }
            std::sort(dists.begin(), dists.end());

            return dists[dists.size() / 2];
        }
};

class AverageCaseEstimator : public IBoundEsimationStrategy{
    private:
        double calcAverageDistance(const GeoPoint& origin, const std::vector<GeoPoint>& data){
            std::vector<double> distances;
			for (size_t i = 0; i < data.size() - 1; i++)
			{
				distances.push_back(abs(data[i].lat - data[i + 1].lat));
			}
            return (std::accumulate(distances.begin(), distances.end(), 0.0) / (data.size() - 1)); //-1 to ignore 'origin'
        }
    public:
        double calcBound(const std::vector<GeoPoint>& data) override {
            std::vector<double> distances;
            for(auto& currentpoint : data){
                distances.push_back(calcAverageDistance(currentpoint, data));
            }
            return (std::accumulate(distances.begin(), distances.end(), 0.0) / data.size());
        }

};

class GodEstimator : public IBoundEsimationStrategy{
    private:
        BestCaseEstimator bestCaseEstimator;
        WorstCaseEstimator worstCaseEstimator;
    public:
        double calcBound(const std::vector<GeoPoint>& data) override{
            return 0.0;
            
        }
};


#endif