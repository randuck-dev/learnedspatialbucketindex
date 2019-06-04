#ifndef QUERY_H
#define QUERY_H

#include <chrono>
#include <vector>
using namespace std::chrono;


enum QueryType{
    SELECT,
    RANGE
};

template <typename T1, typename T2, typename T3>
struct Query{ 
    QueryType qType;
    T1 dim_1_min;
    T1 dim_1_max;

    T2 dim_2_min;
    T2 dim_2_max;

    T3 dim_3_min;
    T3 dim_3_max;

    //Consider to move ctors to .cpp implementation file to keep header free
    Query(T1 dim_1_min_, T1 dim_1_max_, T2 dim_2_min_, T2 dim_2_max_, T3 dim_3_min_, T3 dim_3_max_) : 
      dim_1_min(dim_1_min_), dim_1_max(dim_1_max_), 
      dim_2_min(dim_2_min_), dim_2_max(dim_2_max_),
      dim_3_min(dim_3_min_), dim_3_max(dim_3_max_),
      qType(RANGE)
    {}

    Query(T1 dim_1_min_, T2 dim_2_min_, T3 dim_3_min_) :
      dim_1_min(dim_1_min_), dim_1_max(dim_1_min_), 
      dim_2_min(dim_2_min_), dim_2_max(dim_2_min_),
      dim_3_min(dim_3_min_), dim_3_max(dim_3_min_),
      qType(SELECT)
    {}
      
};

//Temp 5d query
template <typename T>
struct Query_5d{ 
    QueryType qType;
    T dim_1_min;
    T dim_1_max;

    T dim_2_min;
    T dim_2_max;

    T dim_3_min;
    T dim_3_max;

    T dim_4_min;
    T dim_4_max;

    T dim_5_min;
    T dim_5_max;

    //Consider to move ctors to .cpp implementation file to keep header free
    Query_5d(T dim_1_min_, T dim_1_max_, T dim_2_min_, T dim_2_max_, T dim_3_min_, T dim_3_max_, T dim_4_min_, T dim_4_max_, T dim_5_min_, T dim_5_max_) : 
      dim_1_min(dim_1_min_), dim_1_max(dim_1_max_), 
      dim_2_min(dim_2_min_), dim_2_max(dim_2_max_),
      dim_3_min(dim_3_min_), dim_3_max(dim_3_max_),
      dim_4_min(dim_4_min_), dim_4_max(dim_4_max_),
      dim_5_min(dim_5_min_), dim_5_max(dim_5_max_),

      qType(RANGE)
    {}

    Query_5d(T dim_1_min_, T dim_2_min_, T dim_3_min_) :
      dim_1_min(dim_1_min_), dim_1_max(dim_1_min_), 
      dim_2_min(dim_2_min_), dim_2_max(dim_2_min_),
      dim_3_min(dim_3_min_), dim_3_max(dim_3_min_),
      qType(SELECT)
    {}
      
};

//Temp 2d query
template <typename T>
struct Query_2d {
	QueryType qType;
	T dim_1_min;
	T dim_1_max;

	T dim_2_min;
	T dim_2_max;

	//Consider to move ctors to .cpp implementation file to keep header free
	Query_2d(T dim_1_min_, T dim_1_max_, T dim_2_min_, T dim_2_max_) :
		dim_1_min(dim_1_min_), dim_1_max(dim_1_max_),
		dim_2_min(dim_2_min_), dim_2_max(dim_2_max_),
		
		qType(RANGE)
	{}

	Query_2d(T dim_1_min_, T dim_2_min_) :
		dim_1_min(dim_1_min_), dim_1_max(dim_1_min_),
		dim_2_min(dim_2_min_), dim_2_max(dim_2_min_),
		qType(SELECT)
	{}

};



template <typename T1, typename T2, typename T3>
struct QueryEngineResult{
    std::vector<std::tuple<T1,T2,T3>> resultVector; //Assumes 3-dim
    std::chrono::nanoseconds::rep executionTime;
};



#endif