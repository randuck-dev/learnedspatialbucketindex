from tensorflow._api.v1.keras import Sequential
from tensorflow._api.v1.keras.layers import Dense
from sklearn.linear_model import LinearRegression

import tensorflow as tf
import numpy as np
from src.kerasify import export_model

def getcomplexmodel(data, labels, verbose, epochs, name, target) -> Sequential:
    # print("Data size: " + str(len(data)))

    # print("No. of classes: " + str(len(labels)))

    labels = np.asarray(labels).astype('float32')

    model = Sequential()
    model.add(Dense(32, activation='relu', input_dim=1))
    model.add(Dense(32, activation='relu'))
    model.add(Dense(target, activation='linear'))
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
    final_mae = history.history['mean_absolute_error'][-1]
    print("MODEL: {:15} | Loss: {:20} | Accuracy: {:20} | MAE: {:20}".format(
        name, final_loss, final_accuracy, final_mae))
    # with open(DATA_PATH + "modelaccuracy.data", "a") as f:
    #     f.write(name + "," + str(final_accuracy) + "\n")
    export_model(model, str(name) + ".model")
    return model


def getlinearmodel(data, labels, verbose, epochs, name, target) -> Sequential:
    # print("Data size: " + str(len(data)))

    # print("No. of classes: " + str(len(labels)))

    labels = np.asarray(labels).astype('float32')

    model = Sequential()
    model.add(Dense(target, activation='linear', input_dim=1))
    model.compile(optimizer='adam',
                  loss='mse',
                  metrics=['mse', 'accuracy', 'mae'])

    history = model.fit(data,
                        labels,
                        verbose=verbose,
                        epochs=epochs,
                        # batch_size=512,
                        # validation_data=(x_val, y_val)
                        )

    final_accuracy = history.history['acc'][-1]
    final_loss = history.history['loss'][-1]
    final_mae = history.history['mean_absolute_error'][-1]
    print("MODEL: {:15} | Loss: {:20} | Accuracy: {:20} | MAE: {:20}".format(
        name, final_loss, final_accuracy, final_mae))
    # with open(DATA_PATH + "modelaccuracy.data", "a") as f:
    #     f.write(name + "," + str(final_accuracy) + "\n")
    # export_model(model, str(name) + ".model")
    return model


def getLinearRegModel(data, labels):
    reg = LinearRegression().fit(data, labels)
    return reg