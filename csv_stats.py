import pandas as pd
import numpy as np
import matplotlib.pyplot as plt

# read in rocketry CSV file output by Arduino

capsule_one_header_names = ["Latitude", "Longitude", "Altitude (MSL)", "Satellites", "Timestamp", "Accel X", "Accel Y", "Accel Z", "Altitude (AGL)", "Gyro X", "Gyro Y", "Gyro Z"]
capsule_two_header_names = ["VOC Reading", "Humidity", "Temperature"]

csv_file_dataframe = pd.read_csv("CAPS_INF.CSV", header = 0)

# checks to see if capsule two columns are in the CSV file
if set(capsule_two_header_names).issubset(csv_file_dataframe.iloc[0]):
    csv_file_dataframe = pd.read_csv("CAPS_INV.CSV", header = 0)
else:
    print("capsule two columns not found, handling alignment...")

# converts latitude output to actual coordinates
normalizedLatitude = csv_file_dataframe["Latitude"] / pow(10, 7)

# converts longitude output to actual coordinates
normalizedLongitude = csv_file_dataframe["Longitude"] / pow(10, 7)

csv_file_dataframe["Altitude (AGL)"].plot(marker = "x")

plt.xlabel("Row Index")
plt.ylabel("Altitude (AGL) (ft)")
plt.title("Plot of Altitude (AGL) vs row index")
plt.show()
