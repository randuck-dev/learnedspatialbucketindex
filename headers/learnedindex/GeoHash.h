//
// Created by darophi on 5/11/19.
//

#ifndef LINEARREG_GEOHASH_H
#define LINEARREG_GEOHASH_H

#include <iostream>
#include <algorithm>
#include "GeoPoint.h"
#include "IBoundEstimationStrategy.h"


/* (Geohash-specific) Base32 map */
const std::string Geohash_base32 = "0123456789bcdefghjkmnpqrstuvwxyz";
std::string encode(const GeoPoint& point, const int& precision){
    double lat = point.lat;
    double lng = point.lng;
    int idx = 0;
    int bit = 0;

    bool even_bit = true;
    std::string geohash = "";

    double latmin = -90; double latmax = 90;
    double lngmin = -180; double lngmax = 180;

    while(geohash.length() < precision){
        if(even_bit){
            double lngmid = (lngmin + lngmax) / 2;
            if(lng >= lngmid){
                idx = idx * 2 + 1;
                lngmin = lngmid;
            }else{
                idx = idx * 2;
                lngmax = lngmid;
            }
        }else{
            double latmid = (latmin + latmax) / 2;
            if(lat >= latmid){
                idx = idx * 2 + 1;
                latmin = latmid;
            }else{
                idx = idx * 2;
                latmax = latmid;
            }
        }
        even_bit = !even_bit;

        if(++bit == 5){
            geohash += Geohash_base32.at(idx);
            bit = 0;
            idx = 0;
        }
    }
    return geohash;

}

std::map<std::string, std::vector<GeoPoint>> groupByHash(const std::vector<GeoPoint>& data, const int& precision, const uint32_t& MinHashPointsThreshold){
    std::vector<std::tuple<std::string, GeoPoint>> hashes;
    for(auto& pt : data){
        hashes.push_back(make_tuple(encode(pt, precision), pt));
    }

    std::map<std::string, std::vector<GeoPoint>> my_map;
    for(auto& h : hashes){
        if(my_map.find(std::get<0>(h)) == my_map.end()){
            std::vector<GeoPoint> vec {std::get<1>(h)};
            my_map.insert(std::pair<std::string, std::vector<GeoPoint>>(std::get<0>(h), vec));
        }
        else{
            my_map[std::get<0>(h)].push_back(std::get<1>(h));
        }
    }

    for(auto& [hash, ptVec] : my_map){
        if(ptVec.size() > MinHashPointsThreshold){
            std::map<std::string, std::vector<GeoPoint>> tmpMap;
            tmpMap = groupByHash(ptVec, hash.length() + 1, MinHashPointsThreshold);
            for(auto& [h, p] : tmpMap)
                my_map.insert(std::pair<std::string, std::vector<GeoPoint>>(h,p));
        }
    }



    return my_map;
}



struct GeoHashDistanceEstimator {
    uint32_t minHashPointsThreshold;
    uint16_t maxPrecision;
    std::map<std::string, double> boundMap; //TODO: consider to change to 'unordered_map'
    GeoHashDistanceEstimator(const uint32_t MinHashPointsThreshold, std::vector<GeoPoint> data, IBoundEsimationStrategy& estimationStrategy) :
            minHashPointsThreshold(MinHashPointsThreshold), maxPrecision(1) {

        auto groupedPts = groupByHash(data, 1, minHashPointsThreshold);
        
        for(auto& [hash, pointVector] : groupedPts){
            if(pointVector.size() > minHashPointsThreshold){ //"Zoom out" hash regions should not use 
                boundMap.insert(std::pair<std::string, double>(hash, 0.0));
                continue;
            }
            if(pointVector.size() <= 1) continue; //Discard single point regions
            //std::cout << hash << ": " << pointVector.size() << std::endl;
            if(hash.length() > maxPrecision)
                maxPrecision = hash.length();
            boundMap.insert(std::pair<std::string, double>(hash, estimationStrategy.calcBound(pointVector)));
        }
        //std::cout << "-------FIRST HASH STEP DONE--------" << std::endl;

        for(auto& [hash, dist] : boundMap){
            if(hash.length() < maxPrecision){
                //std::cout << hash << std::endl;
                WorstCaseEstimator wce; 
                double worst = -1;
                bool hasChildren = false;
                for(auto& c : Geohash_base32){
                    std::string childHash = hash + c;
                    //std::cout << childHash << std::endl;

                    std::map<std::string, std::vector<GeoPoint>>::const_iterator it = groupedPts.find(childHash);
                    if(it != groupedPts.end()){
                        hasChildren = true;
                        double tmp = wce.calcBound(it->second);
                        if(tmp > worst)
                            worst = tmp;
                    }
                }
                if(hasChildren)
                    boundMap[hash] = worst;
            }
        }

        // for(auto& [hash, bound] : boundMap){
        //     std::cout << hash << ": " << bound << std::endl;
        // }

    };

    double getBound(const GeoPoint& point) {
        std::string hash = encode(point, maxPrecision);
        return getBoundHelper(hash);
    }
    double getBoundHelper(std::string& hash){
        //td::cout << "------------" << std::endl;
        while(true){
          //std::cout << hash << std::endl;
            if(hash.length() == 0) return -1.0; //What do we do in this case?
            std::map<std::string, double>::const_iterator it = boundMap.find(hash);
            if(it == boundMap.end()){
                hash = hash.substr(0, hash.length() - 1);
			}
			else {
					return it->second;
			}
        }

        //recursive version will invalidate iterator
        // if(it == boundMap.end()){
        //     getBound(hash.substr(0, hash.length() - 1)); //truncate hash..
        // }
        // //std::cout << it->second << std::endl;
        // return it->second;
    }
};

#endif //LINEARREG_GEOHASH_H
