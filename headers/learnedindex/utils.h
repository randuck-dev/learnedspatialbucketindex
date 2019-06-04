#pragma once

#include <string>
#include <vector>
#include "keras_model.h"
#include <map>
#include <limits>
#include <learnedindex/csv.h>
#include "GeoPoint.h"

namespace utils {
	time_t string_to_unix(const std::string& value, const char* format);
	KerasModel load_model(const std::string& model_filename);
	std::vector<std::vector<double>> loadSpatialData(const std::string& filename);
	void sizeOfDashboard();
	
	std::map<double, std::vector<double>> getXMappedToY(std::vector<std::vector<double>>& rows);
	double euclideanDistance(const std::pair<double, double>& a, const std::pair<double, double>& b);
	double euclideanDistance(const GeoPoint& pt1, const GeoPoint& pt2);

	template <typename T>
	std::vector<std::pair<T, T>> load_queries(const std::string& filename, const int& instancecount = 30000000) {
		std::vector<std::pair<T, T>> queries;
		io::CSVReader<2> in(filename);
		int linecount = 0;
		T lat, lng;
		while (true) {
			try
			{
				bool read = in.read_row(lat, lng);
				if (!read)
					break;
				queries.push_back(make_pair(lat, lng));
				linecount++;
				if (linecount == instancecount)
					break;
			}
			catch (const std::exception & e)
			{
				std::cerr << e.what() << '\n';
			}
		}
		return queries;
	}
}