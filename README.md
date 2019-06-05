# Learned Spatial Bucket Index

## Requirements

- Libspatialindex
    - https://libspatialindex.org/
    - IF WINDOWS
      - Located in the lib folder are prebuilt binaries that have to be placed next to the executables
-  C++17
- CMake 3.12 or newer
- Optional:
    - Python 3
        - Tensorflow
        - Pandas
        - Numpy
        - Scikit-Learn

## Install
- `mkdir build && cd build && cmake ../`

### [Optional] Python model training
- From root `cd python && pip install -r requirements.txt`
  - Installs all dependencies

## Run

### Training a model

- `python trainmodel.py {DATASET-PATH} {BUCKET-CONF}`
	- In order to process the dataset (csv), it has to have a header containing `LAT,LON`, representing the 2-dimensional data 

### Testing the runtime for a given dataset
- `./bucket {DATASET-PATH} {QUERYSET-PATH} {TOPMODEL-PATH} {BUCKET-CONFIG} {RANGE-QUERY-BOUNDS}` 
    - In order to process the dataset (csv) & query dataset (csv), they have to have a header containing `LAT,LON`, representing the 2-dimensional data
- `./rtree {DATASET-PATH} {DATASET-PATH} {RANGE-QUERY-BOUNDS}`
  - In order to process the dataset (csv) & query dataset (csv) , they have to have a header containing `LAT,LON`, representing the 2-dimensional data
