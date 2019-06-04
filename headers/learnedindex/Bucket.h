#ifndef BUCKET_H
#define BUCKET_H
#include <vector>
#include <stdexcept>
#include "LinearModel.h"
#include "Model.h"

template<typename T>
struct Bucket {
	T m_id; 
	T upper;
	T lower;
	T children_lower;
	T children_upper;
    IModel* m_model;
    std::vector<T> m_data;
	unsigned short layer;
    std::vector<Bucket<T>> m_children;

	friend std::ostream &operator<<(std::ostream &os, const Bucket<T> &p)
	{
		return os << "---- Bucket ----" << "\n >>> ID: " << p.m_id << "\n >>> #Data: " << p.m_data.size() << "\n >>> #Children: " << p.m_children.size() << "\n >>> LG Model: " << p.m_model;
	}

	// Checks whether this container contains a given value
	bool contains(T val, T BOUND = 0) const {
		return val <= upper + BOUND && val >= lower - BOUND;
	}
};


template<typename T>
struct BucketX {
	T m_id;
	std::vector<T> m_data;
};
#endif