from mpl_toolkits import mplot3d
import numpy as np
import matplotlib.pyplot as plt

xline = []
yline = []
zline = []
with open("binaries/bin/data.txt") as f:
    # Parse all the data
    for line in f.readlines():
        d = line.split(' ')
        xline.append(float(d[0]))
        yline.append(float(d[1]))
        zline.append(float(d[2]))
fig = plt.figure()
ax = plt.axes(projection='3d')

# Data for a three-dimensional line
ax.plot3D(xline, yline, zline, 'gray')

plt.show()