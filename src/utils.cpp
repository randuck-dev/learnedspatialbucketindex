
#ifndef UTILS_CPP
#define UTILS_CPP


#include <vector>   
//#include <filesystem>
#include <set>
#include <string.h> 
#include "learnedindex/keras_model.h"
#include "learnedindex/LinearModel.h"
#include "learnedindex/csv.h"
#include "learnedindex/Bucket.h"
#include "learnedindex/utils.h"

//namespace fs = std::filesystem;
using namespace std;

namespace utils {
	class INormalizer {
	public:
		virtual std::vector<double> normalize(std::vector<double> data) = 0;
		virtual ~INormalizer() = default;
	};

	class MinMaxNormalizer : public INormalizer
	{
		double minMaxNormalize(double value, double max_value, double min_value) {
			if (value < min_value)
				return 0;
			if (value > max_value)
				return 1;
			return((value - min_value) / (max_value - min_value));
		}


		std::vector<double> normalize(std::vector<double> data) {
			std::vector<double> normalizedVec;
			const auto [min, max] = minmax_element(data.begin(), data.end());
			for (auto& entry : data)
				normalizedVec.push_back(minMaxNormalize(entry, (double)* max, (double)* min));

			return normalizedVec;
		}
	};

	template<typename T, int NROWS, uint32_t NCOLS, typename ...S>
	std::vector<std::vector<T>> load_data(const std::string& filename, S...colnames) {
		io::CSVReader<sizeof...(S)> in(filename);
		in.read_header(io::ignore_extra_column, std::forward<S>(colnames)...);
		std::vector<std::vector<T>> data;

		while (in.read_row()) {
			std::cout << in.next_line() << std::endl;
		}
	}

	template <typename T>
	double minMaxNormalize(T value, T max_value, T min_value) {
		if (value < min_value)
			return 0;
		if (value > max_value)
			return 1;
		return((value - min_value) / (max_value - min_value));
	}

	template <typename T>
	std::vector<double> minMaxNormalize(const std::vector<T> & data) {
		vector<double> normalizedVector;

		const auto [min, max] = minmax_element(data.begin(), data.end());

		for (auto& entry : data) {
			normalizedVector.push_back(minMaxNormalize(entry, (double)* max, (double)* min));
			//normalizedVector.push_back((entry - *min) / (*max - *min));
		}
		return normalizedVector;
	}




	char* unix_to_readable(const time_t & value) {
		return ctime(&value);
	}

//	auto get_filename_only = [](filesystem::path in) { return in.relative_path().filename().string(); };
//
//	vector<filesystem::path> getFilenames(const string & path) {
//		vector<filesystem::path> filenames;
//		for (const auto& entry : fs::directory_iterator(path))
//			filenames.push_back(entry.path());
//		return filenames;
//	}


	template<typename T>
	vector<T> get_unique_vector(const vector<T> & in) {
		//Investigate if 'std::unique' function is cheaper in form of performance compared to 'set' trick
		//Not of utter most importance since this is pre-processing (However, huge datasets pre-processing might benefit from it)
		set<T> unique_set;
		for (auto& entry : in) {
			unique_set.insert(entry);
		}
		vector<T> out(unique_set.size());
		copy(unique_set.begin(), unique_set.end(), out.begin());
		return out;
	}

	void extract_weight_bias(const KerasModel & model, LinearModel & lm) {
		//Extract and set 'weight' and 'bias' for 'LinearModel'
		for (KerasLayer* i : model.getLayers()) {
			for (auto& w : i->getWeights().data_) {
				lm.weight = w;
			}
			for (auto& b : i->getBiases().data_) {
				lm.bias = b;
			}
		}
	}


	vector<KerasModel> load_model_range(const string & path, const bool& sortNames) {
		KerasModel tmp_model;
		vector<KerasModel> modelVec;
//		vector<filesystem::path> filepaths = utils::getFilenames(path);
//		if (filepaths.empty())
//			return modelVec;
//
//		if (sortNames)
//			sort(filepaths.begin(), filepaths.end());
//
//		for (filesystem::path& filename : filepaths) {
//			tmp_model = load_model(filename.string());
//			modelVec.push_back(tmp_model);
//		}

		return modelVec;
	}
}


KerasModel utils::load_model(const string & model_filename) {
	KerasModel m;
	m.LoadModel(model_filename);
	return m;
}

time_t utils::string_to_unix(const std::string & value, const char* format) {
	struct tm t;
	memset(&t, 0, sizeof(struct tm));
	//strptime(value.c_str(), format, &t);
	return mktime(&t);

}


std::vector<std::vector<double>> utils::loadSpatialData(const std::string& filename) {
	io::CSVReader<2> in(filename);
	in.read_header(io::ignore_extra_column, "LAT", "LON");
	std::vector<std::vector<double>> items;
	double lat, lng;
	int i = 0;
	while (in.read_row(lat, lng)) {
		items.push_back({lat, lng });
		//if (i == 100000) {
		//	break;
		//}
		i++;
		
	}
	std::cout << "[UTILS] Size of DATA: " << items.size() << std::endl;
	return items;
}

void utils::sizeOfDashboard() {
	std::cout << "UINT32_T* " << sizeof(uint32_t*) << std::endl;
	std::cout << "UINT32_T " << sizeof(uint32_t) << std::endl;
	std::cout << "SIZEOF Linearmodel " << sizeof(LinearModel) << std::endl;
	std::cout << "SIZEOF std::vector<Bucket<T>>" << sizeof(std::vector<Bucket<uint32_t>>) << std::endl;
	std::cout << "SIEZOF std::vector<T>>" << sizeof(std::vector<uint32_t>) << std::endl;
	std::cout << "SIZEOF uint32_t " << sizeof(uint32_t) << std::endl;
	std::cout << "SIZEOF Bucket " << sizeof(Bucket<uint32_t>) << std::endl;
	std::cout << "SIZEOF BucketX " << sizeof(BucketX<uint32_t>) << std::endl;
}


std::map<double, std::vector<double>> utils::getXMappedToY(std::vector<std::vector<double>>& rows)
{
	std::map<double, std::vector<double>> XtoY;
	// Iterate through all points

	uint32_t count = 0;
	for (auto& val : rows) {
		auto search = XtoY.find(val[0]);
		if (search == XtoY.end()) { // 
			XtoY.insert(std::make_pair(val[0], std::vector<double>()));
		}	
		XtoY[val[0]].emplace_back(val[1]);
	}

	for (auto& pair : XtoY) {
		count += pair.second.size();
	}
	std::cout << "YCOUNT: " << count << std::endl;

	return XtoY;
}

double utils::euclideanDistance(const std::pair<double, double>& a, const std::pair<double, double>& b)
{
	double x = a.first - b.first; //calculating number to square in next step
	double y = a.second - b.second;
	double dist;

	dist = pow(x, 2) + pow(y, 2);       //calculating Euclidean distance
	dist = sqrt(dist);

	return dist;
}

double utils::euclideanDistance(const GeoPoint& pt1, const GeoPoint& pt2){
    return sqrt(pow(pt1.lat - pt2.lat, 2) + pow(pt1.lng - pt2.lng, 2));
}

#endif