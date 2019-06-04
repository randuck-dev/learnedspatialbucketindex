


#include <iostream>
#if __linux__
#include <spatialindex/capi/sidx_api.h>
#include <spatialindex/capi/sidx_impl.h>
#include <spatialindex/capi/sidx_config.h>
#elif _WIN32
#include <extlib/spatialindex/capi/sidx_api.h>
#include <extlib/spatialindex/capi/sidx_impl.h>
#include <extlib/spatialindex/capi/sidx_config.h>
#endif
#include <ctime>
#include <time.h>
#include <learnedindex/utils.h>
#include <chrono>
#include <random>

using namespace std;
using namespace std::chrono;
using namespace SpatialIndex;

// function to create a new spatial index
Index* createIndex()
{
	// create a property set with default values.
	// see utility.cc for all defaults  http://libspatialindex.github.io/doxygen/Utility_8cc_source.html#l00031
	Tools::PropertySet* ps = GetDefaults();

	Tools::Variant rtreeVar;

	// set index type to R*-Tree
	rtreeVar.m_varType = Tools::VT_ULONG;
	rtreeVar.m_val.ulVal = RT_RTree;

	ps->setProperty("IndexType", rtreeVar);


	Tools::Variant memVar;
	// Set index to store in memory (default is disk)
	memVar.m_varType = Tools::VT_ULONG;
	memVar.m_val.ulVal = RT_Memory;
	ps->setProperty("IndexStorageType", memVar);

	// initalise index
	Index* idx = new Index(*ps);
	delete ps;

	// check index is ok
	if (!idx->index().isIndexValid())
		throw "Failed to create valid index";
	else
		cout << "Index Created" << endl;

	return idx;
}

// add a Point to index.
void addPoint(Index* idx, double lat, double lon, int64_t id)
{
	// create array with lat/lon points
	double coords[] = { lat, lon };

	// shapes can also have an object associated with them but we'll leave that for the moment.
	uint8_t* pData = 0;
	uint32_t nDataLength = 0;

	// create shape
	SpatialIndex::IShape* shape = 0;
	shape = new SpatialIndex::Point(coords, 2);

	// insert into index along with the an object and an ID
	idx->index().insertData(nDataLength, pData, *shape, id);

	//cout << "Point " << id << " inserted into index." << endl;

	delete shape;
}

std::vector<SpatialIndex::IData*>* getPoint(Index* idx, double lat, double lon)
{
	double coords[] = { lat,lon };

	ObjVisitor* visitor = new ObjVisitor;
	SpatialIndex::Point* r = new SpatialIndex::Point(coords, 2);

	idx->index().pointLocationQuery(*r, *visitor);

	int64_t nResultCount;
	nResultCount = visitor->GetResultCount();

	std::vector<SpatialIndex::IData*>& results = visitor->GetResults();
	vector<SpatialIndex::IData*>* resultsCopy = new vector<SpatialIndex::IData*>();
	resultsCopy->push_back(dynamic_cast<SpatialIndex::IData*>(results[0]->clone()));


	delete r;
	delete visitor;
	//cout << "found " << nResultCount << " results." << endl;

	return resultsCopy;
}

std::vector<SpatialIndex::IData*>* getNearest(Index* idx, double lat, double lng, double maxResults) {
	double coords[] = { lat,lng };

	ObjVisitor* visitor = new ObjVisitor;

	SpatialIndex::Point* r = new SpatialIndex::Point(coords, 2);

	idx->index().nearestNeighborQuery(maxResults, *r, *visitor);

	int64_t nResultCount;
	nResultCount = visitor->GetResultCount();
	std::vector<SpatialIndex::IData*>& results = visitor->GetResults();

	vector<SpatialIndex::IData*>* resultCopy = new vector<SpatialIndex::IData*>();

	for (int64_t i = 0; i < nResultCount; ++i) {
		resultCopy->push_back(dynamic_cast<SpatialIndex::IData*>(results[i]->clone()));
	}
	delete r;
	delete visitor;
	return resultCopy;
}

std::vector<long long> r_totals;

uint32_t rtreeRangeQuery(ISpatialIndex* tree, double lat, double lng, double bound) {
	double coords1[] = { lat - bound,    lng - bound };
	double coords2[] = { lat + bound, lng + bound };


	int dims = 2;
	SpatialIndex::Point pt1 = SpatialIndex::Point(coords1, dims);
	SpatialIndex::Point pt2 = SpatialIndex::Point(coords2, dims);

	ObjVisitor* visitor = new ObjVisitor;
	SpatialIndex::Region* r = new SpatialIndex::Region(pt1, pt2);

	high_resolution_clock::time_point t_start = high_resolution_clock::now();
	tree->intersectsWithQuery(*r, *visitor);
	high_resolution_clock::time_point t_end = high_resolution_clock::now();
	std::chrono::nanoseconds::rep total = duration_cast<nanoseconds>(t_end - t_start).count();
	r_totals.push_back(total);
	//cout << "Execution time: " << total << endl;

	int64_t nResultCount;
	nResultCount = visitor->GetResultCount();
	std::vector<SpatialIndex::IData*> & results = visitor->GetResults();
	vector<SpatialIndex::IData*> * resultCopy = new vector<SpatialIndex::IData*>();

	for (int64_t i = 0; i < nResultCount; ++i) {
		resultCopy->push_back(dynamic_cast<SpatialIndex::IData*>(results[i]->clone()));
	}
	std::vector<std::pair<float, float>> res;
	/*for (auto& val : results) {
		IShape* pS;
		val->getShape(&pS);
		const IShape& s = *pS;
		const Point* ppt = dynamic_cast<const Point*>(&s);
		const Region* pr = dynamic_cast<const Region*>(&s);
		std::cout << "(" << pr->getHigh(0) << ", " << pr->getLow(1) << ")" << " : (" << pr->getHigh(1) << ", " << pr->getLow(0) << ")" << std::endl;
	}*/
	//std::cout << "SIZE: " << resultCopy->size() << std::endl;

	delete r;
	delete visitor;
	uint32_t size = resultCopy->size();

	for (auto val : *resultCopy) {
		delete val;
	}
	delete resultCopy;

	return size;
}

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

int main(int argc, char* argv[])
{
	// initalise Indexpointer
	Index* idx = createIndex();
	
	std::string datasetpath = argv[1];
	std::string queryDataset = argv[2];
	std::string rangeboundspath = argv[3];

	std::cout << "Datasetpath: " << datasetpath << "\n";
	std::cout << "Querysetpath: " << queryDataset << "\n";
	std::cout << "Rangebounds: " << rangeboundspath << "\n";


	std::vector<std::vector<double>> data = utils::loadSpatialData(datasetpath);
	auto queries = utils::load_queries<double>(queryDataset);
	std::cout << "Data size: " << data.size() << std::endl;
	std::cout << "Populating R-tree..." << std::endl;
	
	auto range_bounds = loadBounds(rangeboundspath);

	int64_t id = 0;
	for (auto& record : data) {
		addPoint(idx, record[0], record[1], id);
		id++;
		if (id % 100000 == 0) {
			std::cout << "Loaded: " << id << std::endl;
		}
	}
	cout << "R-tree populated!" << endl;


	cout << "Executing NN Queries..." << endl;

	std::vector<long long> nntotals;
	std::vector<uint32_t> nn_sizes;

	// NN QUERIES
	high_resolution_clock::time_point t1 = high_resolution_clock::now();

	for (auto& id : queries) {
		std::vector<SpatialIndex::IData*>* results = getNearest(idx, id.first, id.second, 1);
		nn_sizes.push_back(results->size());
	}
	high_resolution_clock::time_point t2 = high_resolution_clock::now();
	std::chrono::nanoseconds::rep total = duration_cast<nanoseconds>(t2 - t1).count();
	nntotals.push_back(total);
	std::vector<uint32_t> range_sizes;
	
	//std::cout << "Executing Range Queries..." << std::endl;
	//// RANGE QUERIES
	//int maxqueries = 1000;
	//for(int i = 0; i < maxqueries; i++) {
	//	
	//	auto id = queries[i];
	//	auto bound = range_bounds[i];
	//	ISpatialIndex* sidx = &idx->index();
	//	auto ressize = rtreeRangeQuery(sidx, id.first, id.second, 1);
	//	range_sizes.push_back(ressize);
	//	
	//}

	std::cout << "[RTREE] NN SIZES: " << std::accumulate(nn_sizes.begin(), nn_sizes.end(), 0ULL) << std::endl;
	std::cout << "[RTREE] RANGE SIZES: " << std::accumulate(range_sizes.begin(), range_sizes.end(), 0ULL) << std::endl;

	std::cout << "[RTREE] NN ELAPSED TOTAL: " << std::accumulate(nntotals.begin(), nntotals.end(), 0ULL) << std::endl;
	std::cout << "[RTREE] RANGE ELAPSED TOTAL: " << std::accumulate(r_totals.begin(), r_totals.end(), 0ULL) << std::endl;
}