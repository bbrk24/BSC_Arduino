import pandas as pd
import numpy as np
from math import sqrt, pow
from matplotlib import pyplot as plt

# read in CSV file

csv_file = pd.read_csv("CAPS_INF.CSV", header = 0)

accel_x = csv_file["Accel X"]
accel_y = csv_file["Accel Y"]
accel_z = csv_file["Accel Z"]

#sampling frequency we set the accelerometer to
SAMP_FREQ = 50

# gravity is a constant to be subtracted off each value in the magnitude calculation
GRAVITY = 9.81

# 50 ms is a good starting value for time integration
DELTA_T = 1 / SAMP_FREQ

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
accelArr = np.array(accel_magnitude)

# This is HEAVILY based of the below 'definite_integral_calculation', but outputs an array of the integral
# at all points, instead of a scalar of one specified value
def arrIntegralCalc(dataArray: np.ndarray, timeArray: np.ndarray):
    numPoints = len(timeArray)
    trapezoidWidth = DELTA_T
    
    integralList = [0] #to be turned into an array later
    for i in range(1, len(timeArray)):
        integralList.append(integralList[-1] + dataArray[i])
    
    integralArray = np.array(integralList)
    
    return integralArray
    

# this is the algorithm for the trapezoidal approach to calulating the definite integral
def definite_integral_calculation(data_array_input: np.ndarray, start: float, stop: float) -> float:
    trapezoid_width = DELTA_T
    startInd = int(start / DELTA_T)
    stopInd = int(stop / DELTA_T)
        
    # vel = (sum of accelerations felt in time period) * step of timeArr
    # simple ex: accel of 20, 21, 22, 23, 24 at times 0.2, 0.4, 0.6, 0.8, 1.0.
    # accels are over 1 second, we can see the average of accel is about 22, assuming a vel of 0,
    # the vel is going to be about 22m/s. so, (sum of accelerations felt in time period) * step of timeArr
    # gives us the expected answer (maybe not exactly, but this simple ex is a sanity check)
    
    # vel =          step    *        (sum)
    result = trapezoid_width * sum(data_array_input[startInd:stopInd])

    return result

timeArr = np.arange(0, 20.0, DELTA_T)
plt.plot(timeArr, accelArr, 'k') #Acceleration is black
plt.plot(timeArr, accelArr, 'k.')
velArr = arrIntegralCalc(accelArr, timeArr)
plt.plot(timeArr, velArr, 'b') #Velocity is blue
plt.plot(timeArr, velArr, 'b.')
plt.grid(True)

#I did not time how long it took the capsule to drop, but it was most definitely less than a second,
# maybe close to a half second, but our sanity check is that the velocity should be less than 9.81 m/s.
# this is because we know it will experience gravity (9.81 m/s^2) for less than a second, so we know the
# velocity will AT MOST be gravity integrated over 1 second, or 9.81 m/s

#Integrating the Catch (shouldn't be super accurate)
#0.8 - 0.88 is when one catch is
vel1 = definite_integral_calculation(accelArr, 0.8, 0.88) #4.62 m/s
#7.64 - 7.72 is the other
vel2 = definite_integral_calculation(accelArr, 7.64, 7.72) #0.33 m/s - this one doesn't seem right

#Integrating the Fall (should be more accurate)
#First fall is 0.4 to 0.8
altVel1 = definite_integral_calculation(accelArr, 0.4, 0.8) #3.66 m/s
#Second fall is 7.2 to 7.64
altVel2 = definite_integral_calculation(accelArr, 7.2, 7.64) #4.16 m/s - these seem a lot closer

#Integrating the catch seemed to yield sporadic results. This may be mitigated with a larger sample speed.
#However, integrating the fall proved much more fruitful, as the results were closer together and yielded us
#less than 9.81 m/s, which is what we expected. For the Payloads fall test we will trust the integration
#over the fall more than the integration over the catch.
