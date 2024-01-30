import pandas as pd
import numpy as np
import matplotlib.pyplot as plt
import pandas

# read in rocketry CSV file output by Arduino

capsule_one_header_names = ["Latitude", "Longitude", "Altitude (MSL)", "Satellites", "Timestamp", "Accel X", "Accel Y", "Accel Z", "Altitude (AGL)", "Gyro X", "Gyro Y", "Gyro Z"]
capsule_two_header_names = ["VOC Reading", "Humidity", "Temperature"]

csv_file_dataframe = pd.read_csv("CAPS_INF.CSV", header = None)

# checks to see if capsule two columns are in the CSV file
if set(capsule_two_header_names).issubset(csv_file_dataframe.iloc[0]):
    csv_file_dataframe = pd.read_csv("CAPS_INV.CSV", header = 0)
else:
    print("capsule two columns not found, handling alignment...")

print(csv_file_dataframe)
