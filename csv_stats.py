import pandas as pd
import numpy as np
import matplotlib.pyplot as plt
from sklearn.metrics import r2_score
from sklearn.linear_model import LinearRegression
from math import sqrt

# read in rocketry CSV file output by Arduino

capsule_one_header_names = ["Latitude", "Longitude", "Altitude (MSL)", "Satellites", "Timestamp", "Accel X", "Accel Y", "Accel Z", "Altitude (AGL)", "Gyro X", "Gyro Y", "Gyro Z"]
capsule_two_header_names = ["VOC Reading", "Humidity", "Temperature"]

csv_file_dataframe = pd.read_csv("CAPS_INF.CSV", header = 0)

# checks to see if capsule two columns are in the CSV file
if set(capsule_two_header_names).issubset(csv_file_dataframe.iloc[0]):
    csv_file_dataframe = pd.read_csv("CAPS_INV.CSV", header = 0)
else:
    print("Capsule two columns not found, handling alignment...")

# converts latitude output to actual coordinates

normalized_latitude = csv_file_dataframe["Latitude"] / pow(10, 7)

# converts longitude output to actual coordinates

normalized_longitude = csv_file_dataframe["Longitude"] / pow(10, 7)

# grabs MSL Altitude

msl_altitude = csv_file_dataframe["Altitude (MSL)"]

# grabs num satellites

num_satelites = csv_file_dataframe["Satellites"]

# grabs the X Acceleration component from CSV file

accel_x = csv_file_dataframe["Accel X"]

# grabs the Y Acceleration component from CSV file

accel_y = csv_file_dataframe["Accel Y"]

# grabs the Z Acceleration component from CSV file

accel_z = csv_file_dataframe["Accel Z"]

# grabs X acceleration for gyro

gyro_x = csv_file_dataframe["Gyro X"]

# grabs Y acceleration for gyro

gyro_y = csv_file_dataframe["Gyro Y"]

# grabs Z acceleration for gyro

gyro_z = csv_file_dataframe["Gyro Z"]

# IMU data plotting

accel_mag = []
for i in range(len(accel_x)):
    x = accel_x.iloc[i]
    y = accel_y.iloc[i]
    z = accel_z.iloc[i]
    accel_mag.append(sqrt(x*x + y*y + z*z))

accel_np_array = np.array(accel_mag)

# sets row index to be expressed in 50 millisecond increments
row_index = np.arange(len(csv_file_dataframe)).reshape(-1, 1) / 50

plt.figure()
plt.plot(row_index, accel_np_array)
plt.title("IMU Acceleration Magnitude (m/s^2)")
plt.xlabel("Time (s)")
plt.ylabel("Acceleration (m/s^2)")
plt.grid(True)
plt.show()

# altitude plotting

altitude_column = "Altitude (AGL)"
row_index = np.arange(len(csv_file_dataframe)).reshape(-1, 1)
altitude_output = csv_file_dataframe[altitude_column].values.reshape(-1, 1)

# Create a linear regression model
altimeter_model_regression = LinearRegression()
altimeter_model_regression.fit(row_index, altitude_output)

# Predict Y values using the model
altimeter_pred = altimeter_model_regression.predict(row_index)

# # Calculate R^2 value and slope
r2 = r2_score(altitude_output, altimeter_pred)
# I have no earthly idea why the slope is defined like this, but this is what the internet said
slope_output = altimeter_model_regression.coef_[0][0]

# Plot the linear regression line
plt.figure() # this creates a new figure to differentiate between altimeter and IMU graphs
plt.plot(row_index, altimeter_pred, color='red', label=f'Linear Regression\nR^2 = {r2:.5f}\nSlope (ft/min) = {slope_output * 60:.5f}')

# Display other plot details
plt.xlabel("Row Index")
plt.ylabel(f"{altitude_column} (ft)")
plt.title(f"Plot of {altitude_column} vs Row Index")
plt.legend()
plt.show()

# Gyro plotting

gyro_mag = []

# gyro_x, gyro_y, and gyro_z all have the same length, so it doesn't matter which variable you use
for i in range(len(gyro_x)):
    x = gyro_x.iloc[i]
    y = gyro_y.iloc[i]
    z = gyro_z.iloc[i]
    gyro_mag.append(sqrt(x*x + y*y + z*z))

gyro_np_array = np.array(gyro_mag)

plt.figure()
plt.plot(row_index, gyro_np_array)
plt.title("Plot of Gyro Acceleration")
plt.xlabel("Time (s)")
plt.ylabel("Acceleration (m/s^2)")
plt.grid(True)
plt.show()

# mean sea level altitude plotting

altitude_msl_column = "Altitude (MSL)"
altitude_msl_output = csv_file_dataframe[altitude_msl_column].values.reshape(-1, 1)

# Create a linear regression model
altimeter_msl_model_regression = LinearRegression()
altimeter_msl_model_regression.fit(row_index, altitude_msl_output)

# Predict Y values using the model
altimeter_msl_pred = altimeter_msl_model_regression.predict(row_index)

# # Calculate R^2 value and slope
r2_msl = r2_score(altitude_msl_output, altimeter_msl_pred)
# I have no earthly idea why the slope is defined like this, but this is what the internet said
slope_msl_output = altimeter_msl_model_regression.coef_[0][0]

# Plot the linear regression line
plt.figure() # this creates a new figure to differentiate between altimeter and IMU graphs
plt.plot(row_index, altimeter_msl_pred, color='red', label=f'Linear Regression\nR^2 = {r2:.5f}\nSlope (ft/min) = {slope_msl_output * 60:.5f}')

# Display other plot details
plt.xlabel("Row Index")
plt.ylabel(f"{altitude_msl_column} (ft)")
plt.title(f"Plot of {altitude_msl_column} vs Row Index")
plt.legend()
plt.show()

# latitude plotting

plt.figure()
plt.plot(row_index, normalized_latitude)
plt.title("Plot of Row Index vs Latitude")
plt.xlabel("Row Index")
plt.ylabel("Latitude")
plt.grid(True)
plt.show()

# Longitude plotting

plt.figure()
plt.plot(row_index, normalized_longitude)
plt.title("Plot of Row Index vs Longitude")
plt.xlabel("Row Index")
plt.ylabel("Longitude")
plt.grid(True)
plt.show()

# Satellite plotting

plt.figure()
plt.plot(row_index, num_satelites)
plt.title("Plot of Row Index vs Satellites")
plt.xlabel("Row Index")
plt.ylabel("Satellites")
plt.grid(True)
plt.show()