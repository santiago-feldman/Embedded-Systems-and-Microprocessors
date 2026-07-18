import matplotlib.pyplot as plt
from matplotlib.animation import FuncAnimation
import random
import numpy as np

MAXTIME = 100

# Create an empty figure and axis
fig, ax = plt.subplots()
x_data, y_data = [0], [0]
line, = ax.plot([], [], 'bo-')

# Update function for the animation
def update(frame, fargs):
    global x_data, y_data, ax, multiplier
    
    last = x_data[-1]
    y_data = np.concatenate((y_data, fargs()))
    x_data = np.concatenate((x_data, range(last+1, len(y_data))))

    x_data = x_data[-MAXTIME:]
    y_data = y_data[-MAXTIME:]

    #print(x_data)
    #print(y_data)

    line.set_data(x_data, y_data)
    return line

def showOsci(readFromUART):
    ani = FuncAnimation(fig, update, fargs=(readFromUART,), blit=True, interval=10)  # interval in milliseconds
    plt.show()