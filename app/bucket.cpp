#include <cstddef>

#include <learnedindex/rtreebulk.h>
#include <learnedindex/utils.h>
#include <learnedindex/BucketCreator.h>
#include <learnedindex/BucketIdx.h>
#include <learnedindex/GeoHash.h>
#include <math.h>


std::vector<double> loadBounds(std::string name) {
	std::vector<double> bounds;
	io::CSVReader<1> in(name);
	in.read_header(io::ignore_extra_column, "bound");
	
	double bound;
	
	while (in.read_row(bound)) {
		bounds.push_back(bound);
	}
	
	return bounds;
}


int main(int argc, char** argv) {

	std::string datasetpath = argv[1];
	std::string queryDataset = argv[2];
	std::string modelpath = argv[3];
	int val = atoi(argv[4]);
	std::string rangeboundspath = argv[5];

	std::cout << "Datasetpath: " << datasetpath << "\n";
	std::cout << "Querysetpath: " << queryDataset << "\n";
	std::cout << "Modelpath: " << modelpath << "\n";
	std::cout << "Configuration: " << val << "\n";
	std::cout << "Rangebounds: " << rangeboundspath << "\n";

	// Data
	auto spatialData = utils::loadSpatialData(datasetpath);
	auto queries = utils::load_queries<double>(queryDataset);

	std::vector<GeoPoint> dataAsGeo;
	for (auto& x : spatialData) {
		dataAsGeo.push_back({ x[0], x[1] });
	}


	IBoundEsimationStrategy* strat = new BestCaseEstimator();
	std::cout << "Hashing ..." << std::endl;
	auto estimator = GeoHashDistanceEstimator(100, dataAsGeo, *strat);
	std::cout << "Done!" << std::endl;
	 
	auto range_bounds = loadBounds(rangeboundspath);

	BucketCreatorV2<double>* xx = new SpatialBucketCreator<double>();
	xx->modelName = modelpath;
	xx->bucketsize = val;
	BucketIndex<double> bidx(1);
	Bucket<double> buck = xx->bucket(spatialData, 0, 1, { "lat" });
	bidx.setRootBucket(&buck);

	int i = 0;
	int success = 0;

	std::vector<long long> bidxRangeTotals;

	std::cout << "Starting Range-Queries..." << std::endl;
	for (auto& q : queries) {
		double lat = q.first;
		double lng = q.second;
		double bound = range_bounds[i];
		
		Query_2d<double> query = {
			lat - bound, lat + bound,
			lng - bound, lng + bound
		};

		high_resolution_clock::time_point t_start = high_resolution_clock::now();
		std::vector<std::pair<double, double>>* bidxRes = bidx.range_query2d(query);
		high_resolution_clock::time_point t_end = high_resolution_clock::now();
		std::chrono::nanoseconds::rep total = duration_cast<nanoseconds>(t_end - t_start).count();
		bidxRangeTotals.push_back(total);

		i++;
		if (i % 1000 == 0) {
			std::cout << i << " " << bidxRes->size() << std::endl;
			break;
		}
		delete bidxRes;
	}
	auto rangetotal = std::accumulate(bidxRangeTotals.begin(), bidxRangeTotals.end(), 0LL);

	std::cout << "[BUCKETIDX] Total time: " << rangetotal << " Average execution time: " << rangetotal / i << std::endl;
	std::cout << "SUCCESS: " << success << std::endl;
	
	success = 0;
	i = 0;
	std::vector<long long> nn_totals;
	std::vector<long long> buckTotals;
	std::cout << "Starting NN-Queries..." << std::endl;
	for (auto& q : queries) {
		double lat = q.first;
		double lng = q.second;


		std::pair<double, double> nnQ = std::make_pair(lat, lng);
		double epsilon = 0.1;
		int n_nearestNeighbors = 1;

		high_resolution_clock::time_point t_start = high_resolution_clock::now();
		double bound = estimator.getBound({ q.first, q.second });
		//		bound = sqrt(pow(bound, 2) + pow(bound, 2));
				//std::cout << bound << std::endl;
		if (bound == 0) {
			bound = 0.1;
		}
		Query_2d<double> rangeQuery2 = {
			lat - bound, lat + bound,
			lng - bound, lng + bound
		};

		// BUCKETINDEX NEAREST
		const auto nns = bidx.nearestNeighborQuery_2d(rangeQuery2, nnQ, epsilon, bound, n_nearestNeighbors);
		const auto nnBidx = nns[0].nn;

		high_resolution_clock::time_point t_end = high_resolution_clock::now();
		std::chrono::nanoseconds::rep t_total = duration_cast<nanoseconds>(t_end - t_start).count();
		buckTotals.push_back(t_total);
		

		i++;
		if (i % 1000 == 0) {
			std::cout << i << std::endl;
		}
	}
	auto nntotal = std::accumulate(buckTotals.begin(), buckTotals.end(), 0LL);

	std::sort(buckTotals.begin(), buckTotals.end());

	auto maxBuckIdxVal = buckTotals[buckTotals.size() - 1];

	std::cout << "[BUCKETIDX] Largest time: " << maxBuckIdxVal << std::endl;
	std::cout << "[BUCKETIDX] Median time: " << buckTotals[buckTotals.size() / 2] << std::endl;
	std::cout << "[BUCKETIDX] Shortest time: " << buckTotals[0] << std::endl;
	std::cout << "[BUCKETIDX] Total time: " << nntotal << " Average execution time: " << nntotal / i << std::endl;
	std::cout << "SUCCESS: " << success << "/" << i << std::endl;

	delete xx;

}
