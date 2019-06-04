import random
import sys
from collections import namedtuple

import numpy as np
import pandas as pd
from tensorflow._api.v1.keras import Sequential
from tensorflow._api.v1.keras.layers import Dense

import src.kerasify as kerasify

Point = namedtuple('Point', ['x', 'y'])

def load_data(fileName, bucketSize):
    
    data = pd.read_csv('..\\data\\' + fileName + '.csv', sep=",",
                       usecols=['LAT', 'LON'])
    data.columns = ['lat', 'lng']
    # data = data.reindex(columns=['lat', 'lng'])
    print(data.describe())
    print(len(data))
    data = data.sort_values(by=['lat']).reset_index()
    remainder = len(data) % bucketSize
    n_iterations = (len(data) - remainder) / bucketSize
    print("LOADED: {}".format(len(data)))
    print("ITERATIONS: {} | REMAINDER: {}".format(n_iterations, remainder))
    buckets = {}
    counter = 0
    bucketId = 0
    min_val = data.ix[0]['lat']
    print(data[-10:])
    max_val = data.ix[len(data)-1]['lat']
    print("MIN: {} MAX: {}".format(min_val, max_val))
    for i in range(0, int(n_iterations)):
        counter = bucketize(data, counter, min_val, max_val, buckets, bucketId, bucketSize)
        bucketId += 1

    if remainder > 0:
        print("INTERMEDIATE COUNTER: {}".format(counter))
        counter = bucketize(data, counter, min_val, max_val, buckets, bucketId, remainder)
        bucketId += 1
    print("COUNTER: {}".format(counter))

    x = []
    y = []
    naive_x = []
    naive_y = []
    for buckId, val in buckets.items():
        # print(val['values'])
        for l in val['values']:
            x.append([l])
            y.append([buckId])
            naive_x.append(l)
            naive_y.append(buckId)

    return x, y, naive_x, naive_y


def bucketize(data, counter, min_val, max_val, buckets, bucketId, n_iterations):
    vals = []
    points = []
    low = data.ix[counter]['lat']  # Get the current bucket LOW
    for j in range(0, n_iterations):
        vals.append(data.ix[counter]['lat'])
        # points.append(Point(data.ix[counter]['lat'], data.ix[counter]['lng']))
        counter += 1
        if(counter % 50000 == 0):
            print("Processed: ", counter)
    high = data.ix[counter-1]['lat']  # Get the current bucket HIGH
    bucket = {
        'values': vals,
        'low ': low,
        'high': high
    }
    buckets[bucketId] = bucket
    # print("Low: {:15} | High: {:15}".format(low, high))
    return counter


def normalize(v_min, v_max, value):
    return (value - v_min) / (v_max - v_min)


def getcomplexmodel(data, labels, verbose, epochs, name) -> Sequential:
    # print("Data size: " + str(len(data)))

    # print("No. of classes: " + str(len(labels)))

    labels = np.asarray(labels).astype('float32')

    model = Sequential()
    model.add(Dense(32, activation='relu', input_dim=1))
    model.add(Dense(32, activation='relu'))
    model.add(Dense(1, activation='linear'))
    model.compile(optimizer='adam',
                  loss='mse',
                  metrics=['mse', 'accuracy', 'mae'])

    history = model.fit(data,
                        labels,
                        verbose=verbose,
                        epochs=epochs,
                        # batch_size=512,
                        #validation_data=(x_val, y_val)
                        )
    final_accuracy = history.history['acc'][-1]
    final_loss = history.history['loss'][-1]
    print("MODEL: {:15} | Loss: {:20} | Accuracy: {:20}".format(
        name, final_loss, final_accuracy))
    # with open(DATA_PATH + "modelaccuracy.data", "a") as f:
    #     f.write(name + "," + str(final_accuracy) + "\n")
    kerasify.export_model(model, name + ".model")
    return model


dataset = sys.argv[1]
conf = sys.argv[2]

print("DATASET: {} | CONF: {}".format(dataset, conf))
x, y, nx, ny = load_data(dataset, conf)
# showUniquePoint(nx, ny)
model = getcomplexmodel(np.array(x), np.array(y), True, 5, "toplevel_{}".format(dataset) + "_" + str(conf))

n = 10
print("Running {} predictions...".format(n))
for i in range(0, n):
    rand = random.randint(0, len(x)-1)
    query = x[rand]
    query_answer = y[rand]
    print("QUERY: {:5} POSITION: {}".format(query, query_answer))
    pred = model.predict(query)
    print("PRED: {:5} INT-PRED: {}".format(
        pred, int(pred)))
    print("-"*30)
