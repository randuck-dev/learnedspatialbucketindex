#ifndef GEOPOINT_H
#define GEOPOINT_H
#include <iostream>

struct GeoPoint {
    double lat;
    double lng;

    friend std::ostream& operator<<(std::ostream& os, const GeoPoint& pt){
        os << "(" << pt.lat << "," << pt.lng << ")";
        return os;
    };

    bool operator==(const GeoPoint& other) const {
        return (lat == other.lat) && (lng == other.lng);
    };
};

#endif