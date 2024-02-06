import pandas as pd
import numpy as np
from math import sqrt, pow

# read in CSV file

csv_file = pd.read_csv("CAPS_INF.CSV", header = 0)

accel_x = csv_file["Accel X"]
accel_y = csv_file["Accel Y"]
accel_z = csv_file["Accel Z"]

# gravity is a constant to be subtracted off each value in the magnitude calculation
GRAVITY = 9.81

# 50 ms is a good starting value for time integration
DELTA_T = 0.02

# calculates magnitude of row values
accel_magnitude = []
for i in range(len(accel_x)):
    x = accel_x.iloc[i]
    y = accel_y.iloc[i]
    z = accel_z.iloc[i]
    magnitude = sqrt(pow(x, 2) + pow(y, 2) + pow(z, 2))
    total_output = magnitude - GRAVITY
    accel_magnitude.append(total_output)

# loads into NumPy array for easier data analysis
np_array = np.array(accel_magnitude)

# this is the algorithm for the trapezoidal approach to calulating the definite integral
def definite_integral_calculation(data_array_input: np.ndarray, start: float, stop: float) -> float:
    num_pts = int((stop - start) / DELTA_T)
    trapezoid_width = DELTA_T

    integral_sum = (data_array_input[start] + data_array_input[stop]) / 2

    for i in range(1, num_pts):
        x_i = start + i * DELTA_T
        integral_sum += data_array_input[int(x_i)]
    
    result = trapezoid_width * integral_sum

    return result