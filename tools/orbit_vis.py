from mpl_toolkits import mplot3d
import numpy as np
import matplotlib.pyplot as plt

xline = [0]
yline = [0]
zline = [0]
data_line = [0]
with open("binaries/bin/data.txt") as f:
    # Parse all the data
    d2 = 0
    # Skip every 3 lines
    for line in f.readlines():
        d = line.split(' ')
        d2 = d2 + 1
        if d2 % 50 != 0:
            continue
        data_line.append(float(d2))
        xline.append(float(d[1]))
        yline.append(float(d[2]))
        zline.append(float(d[3]))



fig = plt.figure()
ax = plt.axes(projection='3d')

# Data for a three-dimensional line
ax.plot3D(xline, yline, zline, 'gray')
ax.scatter3D(xline, yline, zline, c=data_line, cmap='Greens')
x = max(max(yline), max(xline))

ax.set_xlim3d(-x, x)
ax.set_ylim3d(-x, x)
ax.set_zlim3d(-x, x)
plt.show()
