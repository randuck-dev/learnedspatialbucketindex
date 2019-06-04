#include <iostream>
#include <vector>
#include <tuple>
#include <string>
#include <fstream>
#include <random>
#include <ctime>
#include <math.h>

struct Range{
    float min;
    float max;
};

struct QueryRange{
    std::vector<Range> Ranges;
};

//loads the dataset 
//can take any number of dimensions, dimension number must however match dataset dimensions
std::vector<std::vector<float>> loadData(const std::string filename, const int dimensions) {
    std::ifstream read_file(filename);
    std::string::size_type sz;
    std::vector<std::vector<float>> data;
    
    std::cout << "Loading data... \n";
    while (read_file.good()) {
        std::vector<float> rowData;
        for(int i = 0; i < dimensions; i++){
            std::string x;
            if(i < dimensions - 1){
                getline(read_file, x, ',');
                if(x.length() != 0)
                    rowData.push_back(std::stof(x, &sz));
            }
            else if(i == dimensions - 1){
                getline(read_file, x, '\n');
                if(x.length() != 0)
                    rowData.push_back(std::stof(x, &sz));
            }
        }
        data.push_back(rowData);
    }
    std::cout << "Loading of data finished!" << std::endl;
    std::cout << "Datasize: " << data.size() << std::endl;
    return data;
}

//finds the range (min & max) of each dimension
QueryRange getRange(const std::vector<std::vector<float>> data, const int dimensions){
    std::vector<float> min;
    std::vector<float> max;

    for(int i = 0; i < data.size(); i++){
        for(int j = 0; j < dimensions; j++){
            if(i == 0){
                min.push_back(data[i][j]);
                max.push_back(data[i][j]);
            }
            else{
                if(data[i][j] > max[j])
                    max[j] = data[i][j];
                if(data[i][j] < min[j])
                    min[j] = data[i][j];
            }
        }
    }
    QueryRange dataRanges;
    for(int i = 0; i < max.size(); i++){
        Range range;
        range.min = min[i];
        range.max = max[i];
        dataRanges.Ranges.push_back(range);
    }
    return dataRanges;
}

//generates the random values for each dimension, either existing values in the dataset or new values in the dimension ranges
std::vector<QueryRange> generateRangeQueries(const std::string filename, const int dimensions, const int q_amount, std::string q_dims){
    if(dimensions < q_dims.length())
        throw "Requested dimensions longer than possible dimensions in dataset!";
    std::vector<int> q_dims_int;
    for(char& ch : q_dims)
        q_dims_int.push_back(ch-'0');

    const std::vector<std::vector<float>> data = loadData(filename, dimensions);

    QueryRange dataRanges = getRange(data, dimensions);
    std::vector<QueryRange> queries;
    srand(time(NULL));

    for(int i = 0; i < q_amount; i++){
        QueryRange query;
        for(int j = 0; j < q_dims_int.size(); j++){
            Range curr_dim_range;
            if(q_dims_int[j] == 1){
                //float a = dataRanges.Ranges[j].min + (rand() / (RAND_MAX/(dataRanges.Ranges[j].max-dataRanges.Ranges[j].min)));
                //float b = dataRanges.Ranges[j].min + (rand() / (RAND_MAX/(dataRanges.Ranges[j].max-dataRanges.Ranges[j].min)));
                int a_index = 0 + (rand() % static_cast<int>(data.size()-1 - 0 + 1));
                int b_index = 0 + (rand() % static_cast<int>(data.size()-1 - 0 + 1));
                float a = data[a_index][j];
                float b = data[b_index][j];
                if(a < b){
                    curr_dim_range.min = a;
                    curr_dim_range.max = b;
                }
                else{
                    curr_dim_range.min = b;
                    curr_dim_range.max = a;
                }
            }
            else{
                curr_dim_range.min = dataRanges.Ranges[j].min;
                curr_dim_range.max = dataRanges.Ranges[j].max;
            }
            query.Ranges.push_back(curr_dim_range);
        }
        queries.push_back(query);
    }
    return queries;
}

//printer function for printing the range queries
void printRangeQueries(std::vector<QueryRange> queries){
    for(int i = 0; i < queries.size(); i++){
        std::cout << "Query " << i << " with ranges: " << std::endl;
        for(int j = 0; j < queries[i].Ranges.size(); j++){
            std::cout << "Dim " << j << ": " << queries[i].Ranges[j].min << " - " << queries[i].Ranges[j].max << std::endl;
        }
    }
}

//hard-coded 5-dim generator, should be deleted at some point.
std::vector<std::tuple<int, int, float, int, int>> generateQueries(const int q_amount, const std::tuple<int,int> ship_dates, const std::tuple<int,int> quantities,
                    const std::tuple<float,float> discounts, const std::tuple<int,int> cust_keys, const std::tuple<int,int> order_dates){
    
    std::vector<std::tuple<int, int, float, int, int>> queries;
    srand(time(NULL));

    for(int i = 0; i < q_amount; i++){
        int rand_ship_date = rand() % (std::get<1>(ship_dates) - std::get<0>(ship_dates) + 1) + std::get<0>(ship_dates);
        int rand_quantity = rand() % (std::get<1>(quantities) - std::get<0>(quantities) + 1) + std::get<0>(quantities);
        float rand_discount = std::get<0>(discounts) + (rand() / (RAND_MAX/(std::get<1>(discounts)-std::get<0>(discounts))));
        int rand_cust_key = rand() % (std::get<1>(cust_keys) - std::get<0>(cust_keys) + 1) + std::get<0>(cust_keys);
        int rand_order_date = rand() % (std::get<1>(order_dates) - std::get<0>(order_dates) + 1) + std::get<0>(order_dates);

        queries.push_back(std::make_tuple(rand_ship_date, rand_quantity, rand_discount, rand_cust_key, rand_order_date));
    }

    return queries;
}

int main(){
    std::vector<QueryRange> data = generateRangeQueries("data.csv", 5, 10, "11111");
    printRangeQueries(data);

    /*std::tuple<int, int> ship_dates = std::make_tuple(694306800, 912466800);
    std::tuple<int, int> quantities = std::make_tuple(1, 50);
    std::tuple<float, float> discounts = std::make_tuple(0, 0.1);
    std::tuple<int, int> customer_keys = std::make_tuple(1, 749999);
    std::tuple<int, int> order_dates = std::make_tuple(694220400, 902008800);

    std::vector<std::tuple<int, int, float, int, int>> queries = generateQueries(100000, ship_dates, quantities,
                                                                                discounts, customer_keys, order_dates);*/
}