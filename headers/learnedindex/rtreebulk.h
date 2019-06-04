#include <ctime> 
#include <time.h> 
#include <chrono> 
#include "learnedindex/csv.h" 
#include <stdexcept>
#if __linux__
#include <spatialindex/capi/sidx_api.h>
#include <spatialindex/capi/sidx_impl.h>
#include <spatialindex/capi/sidx_config.h>
#elif _WIN32
#include <extlib/spatialindex/capi/sidx_api.h>
#include <extlib/spatialindex/capi/sidx_impl.h>
#include <extlib/spatialindex/capi/sidx_config.h>
#endif

using namespace std;
using namespace std::chrono;
using namespace SpatialIndex;




class DStream : public IDataStream {
public:
	DStream(std::string inFile) : m_pNext(nullptr), m_counter(0) {
		m_fin.open(inFile.c_str());
		if (!m_fin)
			throw std::invalid_argument("Check file path file path!");

		//HACK TO AVOID HEADER
		std::string line;
		std::getline(m_fin, line);


		readNextEntry();
	}

	~DStream() override {
		if (m_pNext != nullptr) delete m_pNext;
	}

	IData* getNext() override {
		if (m_pNext == nullptr) return nullptr;

		RTree::Data * ret = m_pNext;
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

	void rewind() override {
		//PLZ NOT BE NEEDED
		if (m_pNext != nullptr) {
			delete m_pNext;
			m_pNext = nullptr;
		}

		m_fin.seekg(0, std::ios::beg);
		readNextEntry();
	}

	void readNextEntry() {
		id_type id = m_counter;
		/*if (m_counter == 100001) {
			return;
		}*/
		m_counter++;
		//if (m_counter % 10000 == 0)
		//	std::cout << m_counter << std::endl;
		std::string line;
		std::getline(m_fin, line);
		std::istringstream s(line);
		std::string field;

		double data[2];

		int i = 0;
		while (getline(s, field, ',')) {
			double tmp = std::stod(field);
			data[i] = tmp;
			i++;
		}
		/*double placeholder = data[0];
		data[0] = data[1];
		data[1] = placeholder;*/
		if (m_fin.good()) {
			SpatialIndex::Region r(data, data, 2);
			m_pNext = new RTree::Data(sizeof(double), reinterpret_cast<uint8_t*>(data), r, id);
		}

	}

	std::ifstream m_fin;
	RTree::Data* m_pNext;
	int m_counter;

};
