import pandas as pd
import numpy as np
import matplotlib.pyplot as plt
from sklearn.metrics import r2_score, mean_squared_error
from sklearn.linear_model import LinearRegression

# read in rocketry CSV file output by Arduino

capsule_one_header_names = ["Latitude", "Longitude", "Altitude (MSL)", "Satellites", "Timestamp", "Accel X", "Accel Y", "Accel Z", "Altitude (AGL)", "Gyro X", "Gyro Y", "Gyro Z"]
capsule_two_header_names = ["VOC Reading", "Humidity", "Temperature"]

csv_file_dataframe = pd.read_csv("CAPS_INF.CSV", header=0)

# checks to see if capsule two columns are in the CSV file
if set(capsule_two_header_names).issubset(csv_file_dataframe.iloc[0]):
    csv_file_dataframe = pd.read_csv("CAPS_INV.CSV", header=0)
else:
    print("Capsule two columns not found, handling alignment...")

# converts latitude output to actual coordinates
normalizedLatitude = csv_file_dataframe["Latitude"] / pow(10, 7)

# converts longitude output to actual coordinates
normalizedLongitude = csv_file_dataframe["Longitude"] / pow(10, 7)

# picks the column data to plot with x's
altitude_column = "Altitude (AGL)"
csv_file_dataframe[altitude_column].plot(marker="x")

# Create X and Y arrays for linear regression
X = np.arange(len(csv_file_dataframe)).reshape(-1, 1)
Y = csv_file_dataframe[altitude_column].values.reshape(-1, 1)

# Create a linear regression model
model = LinearRegression()
model.fit(X, Y)

# Predict Y values using the model
Y_pred = model.predict(X)

# Calculate R^2 value and MSE
r2 = r2_score(Y, Y_pred)
mse = mean_squared_error(Y, Y_pred)

# Plot the linear regression line
plt.plot(X, Y_pred, color='red', label=f'Linear Regression\nR^2 = {r2:.2f}\nMSE = {mse:.2f}')

# Display other plot details
plt.xlabel("Row Index")
plt.ylabel(f"{altitude_column} (ft)")
plt.title(f"Plot of {altitude_column} vs Row Index")
plt.legend()
plt.show()
