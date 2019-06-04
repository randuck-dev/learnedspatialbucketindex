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
#include <chrono> 
// #include "../load.cpp" 
#include "learnedindex/csv.h" 
#include <stdexcept>


using namespace std;
using namespace std::chrono;
using namespace SpatialIndex;


class DStream : public IDataStream{
    public:
        DStream(std::string inFile) : m_pNext(nullptr), m_counter(0) {
            m_fin.open(inFile.c_str());
            if(!m_fin)
                throw std::invalid_argument("");

            //HACK TO AVOID HEADER
            std::string line;
            std::getline(m_fin,line);


            readNextEntry();
        }

        ~DStream() override{
            if(m_pNext != nullptr) delete m_pNext;
        }

        IData* getNext() override{
            if(m_pNext == nullptr) return nullptr;

            RTree::Data* ret = m_pNext;
            m_pNext = nullptr;
            readNextEntry();
            return ret;            
        }

        bool hasNext() override {
            return (m_pNext != nullptr);
        }

        uint32_t size() override
	    {
		    throw Tools::NotSupportedException("Operation not supported.");
            //PLZ
	    }

        void rewind() override{
            //PLZ NOT BE NEEDED
            if(m_pNext != nullptr){
                delete m_pNext;
                m_pNext = nullptr;
            }

            m_fin.seekg(0, std::ios::beg);
            readNextEntry();
        }

        void readNextEntry(){
            id_type id = m_counter;
            m_counter++;
            if(m_counter % 1000000 == 0)
                std::cout << m_counter << std::endl;
            std::string line;
            std::getline(m_fin,line);
            std::istringstream s(line);
            std::string field;

            double data[5];

            int i = 0;
            while (getline(s, field,',')){
                int tmp;
                double d;
                if(i == 4){
                    d = std::stod(field);
                    tmp = d*100;
                }
                else
                    tmp = std::stoi(field);
                data[i] = tmp;
                i++;
            }

            if(m_fin.good()){
			    SpatialIndex::Region r(data, data, 5);
                m_pNext = new RTree::Data(sizeof(double), reinterpret_cast<uint8_t*>(data), r, id);
            }
            
        }

        std::ifstream m_fin;
        RTree::Data* m_pNext;
		int m_counter;

};


int main(int argc, char const *argv[])
{
    IStorageManager* memStorage = StorageManager::createNewMemoryStorageManager();
    StorageManager::IBuffer* file = StorageManager::createNewRandomEvictionsBuffer(*memStorage,10,false);

    DStream stream("15m.csv");

    id_type idxID;

    ISpatialIndex* tree = RTree::createAndBulkLoadNewRTree(RTree::BLM_STR, 
                                                           stream, 
                                                           *file,
                                                           0.7, //fillfactor (Default 0.7)
                                                           100, //indexCapacity (Default 100)
                                                           100, //leafCapacity (Default 100)
                                                           5,   //Dimensions
                                                           SpatialIndex::RTree::RV_RSTAR,
                                                           idxID);



    bool valid = tree->isIndexValid();
    if (valid == false) 
        std::cerr << "ERROR: Structure is invalid!" << std::endl;
    else std::cerr << "RTree structure is: valid." << std::endl;

    double coords1[] = {1,    714441600,720316800, 1, 1};
	double coords2[] = {199500,908572800,892979200, 50,10};


    int dims = 5;
    SpatialIndex::Point pt1 = SpatialIndex::Point(coords1, dims);
	SpatialIndex::Point pt2 = SpatialIndex::Point(coords2, dims);

    ObjVisitor* visitor = new ObjVisitor;
	SpatialIndex::Region* r = new SpatialIndex::Region(pt1, pt2);
    
    high_resolution_clock::time_point t_start = high_resolution_clock::now();    
    tree->intersectsWithQuery(*r, *visitor);
    high_resolution_clock::time_point t_end = high_resolution_clock::now();
	std::chrono::nanoseconds::rep total = duration_cast<nanoseconds>(t_end - t_start).count();
    cout << "Execution time: " << total << endl;

    int64_t nResultCount;
	nResultCount = visitor->GetResultCount();
	std::vector<SpatialIndex::IData*>& results = visitor->GetResults();
	vector<SpatialIndex::IData*>* resultCopy = new vector<SpatialIndex::IData*>();

	for(int64_t i = 0; i < nResultCount; ++i){
		resultCopy->push_back(dynamic_cast<SpatialIndex::IData*>(results[i]->clone()));
	}


	cout << resultCopy->size() << endl;
	delete r;
	delete visitor;


    return 0;
}
