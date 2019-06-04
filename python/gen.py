import random
import pandas as pd
def load_data(instance_count: int):
    
    data = pd.read_csv('../data/c_open_addresses_10m.csv', sep=",",
                       usecols=['lat', 'lng'])
    data.columns = ['lat', 'lng']
    

    data = data.sort_values(by=['lat']).reset_index()
    return data
    
openData = load_data(100000000)

data_lower = 0
data_upper = len(openData)-1
with open('../data/query_nearest_open_10m.data', 'w') as outf:
    for i in range(0, 5000):
        valIdx = random.randint(data_lower, data_upper)
        
        lng = openData.iloc[valIdx]['lng']
        lat = openData.iloc[valIdx]['lat']
        lower = -0.1
        upper = 0.1
        latVal = lat + random.uniform(lower, upper)
        lngVal = lng + random.uniform(lower, upper)
        outf.write('{},{}\n'.format(latVal, lngVal))