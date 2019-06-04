
#ifndef LINEARREG_LG_H
#define LINEARREG_LG_H

#include<stdlib.h>
#include<stdbool.h>
#include<stdio.h>
#include<string.h>
#include<math.h>
#include "LinearModel.h"


static double arithmetic_mean(const std::vector<double> &data, size_t size);

static double mean_of_products(const std::vector<double> &data1, const std::vector<double> &data2, size_t size);

static double variance(const std::vector<double> &data, size_t size);

// --------------------------------------------------------
// FUNCTION linear_regression
// --------------------------------------------------------
void linear_regression(const std::vector<double> &independent, const std::vector<double> &dependent, size_t size,
                       LinearModel &lr) {
    double independent_mean = arithmetic_mean(independent, size);
    double dependent_mean = arithmetic_mean(dependent, size);
    double products_mean = mean_of_products(independent, dependent, size);
    double independent_variance = variance(independent, size);

    lr.weight = (products_mean - (independent_mean * dependent_mean)) / independent_variance;

    lr.bias = dependent_mean - (lr.weight * independent_mean);
}

//--------------------------------------------------------
// FUNCTION arithmetic_mean
//--------------------------------------------------------
static double arithmetic_mean(const std::vector<double> &data, size_t size) {
    double total = 0;

    // note that incrementing total is done within the for loop
    for (uint32_t i = 0; i < size; total += data[i], i++);

    return total / size;
}

//--------------------------------------------------------
// FUNCTION mean_of_products
//--------------------------------------------------------

static double mean_of_products(const std::vector<double> &data1, const std::vector<double> &data2, size_t size) {
    double total = 0;

    // note that incrementing total is done within the for loop
    for (uint32_t i = 0; i < size; total += (data1[i] * data2[i]), i++);

    return total / size;
}

//--------------------------------------------------------
// FUNCTION variance
//--------------------------------------------------------
static double variance(const std::vector<double> &data, size_t size) {
    std::vector<double> squares(size);

    for (uint32_t i = 0; i < size; i++) {
        squares[i] = pow(data[i], 2);
    }

    double mean_of_squares = arithmetic_mean(squares, size);
    double mean = arithmetic_mean(data, size);
    double square_of_mean = pow(mean, 2);
    double variance = mean_of_squares - square_of_mean;

    return variance;
}

#endif