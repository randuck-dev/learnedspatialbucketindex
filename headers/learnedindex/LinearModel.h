#ifndef LINEARMODEL_H
#define LINEARMODEL_H

#include <iostream>
#include "Model.h"
struct LinearModel: public IModel
{
	double weight{0};
	double bias{0};

	virtual float infer(const float &input) override
	{
		return (input * weight) + bias;
	}

	friend std::ostream &operator<<(std::ostream &os, const LinearModel &p)
	{
		return os << "bias: " << p.bias << " weight: " << p.weight;
	}
};

#endif