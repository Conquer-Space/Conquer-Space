# Generates a series of points on a sphere, then generates a vornoi diagram based of it, and outputs it onto an image
import numpy as np
from scipy.spatial import SphericalVoronoi, geometric_slerp
import matplotlib.pyplot as plt
import collections
from PIL import Image

def generate_random_points(count: int, seed: int) -> np.ndarray:
    """
    Generate a series of latitude and longitude points
    https://mathworld.wolfram.com/SpherePointPicking.html
    """
    np.random.seed(seed)
    points = np.random.rand(count, 3) * 2 - 1
    return 1 / np.linalg.norm(points, axis=1)[:,None] * points

def to_latitude_longitude(points: np.ndarray) -> np.ndarray:
    # Convert 3d point to latitude and longitude
    theta = np.arctan2(points[:,1], points[:,0])
    phi = np.arctan2(np.sqrt(points[:,1]**2 + points[:,0]**2), points[:,2])
    return np.vstack((theta, phi)).T

def lat_lon_to_image(points: np.ndarray):
    x_points = np.round(((points[:,0]) / np.pi + 1) * (output_shape[0] // 2))
    y_points = np.round(((points[:,1]) / (np.pi)) * (output_shape[1]))
    x_points = np.clip(x_points, a_min=0, a_max=output_shape[0] - 1)
    y_points = np.clip(y_points, a_min=0, a_max=output_shape[1] - 1)
    x_points = x_points.astype(np.int16)
    y_points = y_points.astype(np.int16)
    return x_points, y_points
    
voronoi_points = generate_random_points(500, 23)
sv = SphericalVoronoi(voronoi_points)
sv.calculate_areas()
t_vals = np.linspace(0, 1, 2000)

output_shape = (1024, 512)
output_image = np.zeros(output_shape)
for region in sv.regions:
   n = len(region)
   for i in range(n):
        start = sv.vertices[region][i]
        end = sv.vertices[region][(i + 1) % n]
        result = geometric_slerp(start, end, t_vals)
        # Now convert all the points to an image
        points = to_latitude_longitude(result)
        x_points, y_points = lat_lon_to_image(points)

        # TODO: Instead of relying on maxing out the t_vals so that we oversaturate the different borders,
        # we should just lerp the points between each vertex
        # I'm not sure if this is a function or we can have multiple outputs for one input
        output_image[x_points, y_points] = 1

print("Done initializing regions!")
# plt.imshow(output_image.T)
# plt.show()

# Now floodfill everything
# I think we should greedly take what is present
# We can assume that each
image = np.zeros((*output_shape, 3))
# We can floodfill starting from the original points
# Color picking...

def is_in_group(output_image, image, point):
    return np.all(image[*point] == np.zeros(3))

coloring = 1
voronoi_points = to_latitude_longitude(voronoi_points)
voronoi_points = np.vstack(lat_lon_to_image(voronoi_points)).T

def add_if_within_bounds(queue: collections.deque, point: np.ndarray) -> None:
    queue.append(np.array([point[0] % output_shape[0], point[1] % output_shape[1]]))

for pt in voronoi_points:
    point_queue = collections.deque()
    point_queue.append(pt)
    district_color = np.array([coloring % 256, coloring // 256 % 256, coloring // (256) // 256])
    idx = 1
    while len(point_queue) > 0:
        n = point_queue.popleft()
        if is_in_group(output_image, image, n):
            # Color the point
            image[*n] = district_color#np.array([1, 0, 0])
            # then we color it and add neighbors
            # Check if it's within the bounds
            if output_image[*n] == 0:
                add_if_within_bounds(point_queue, n + np.array([1, 0]))
                add_if_within_bounds(point_queue, n + np.array([-1, 0]))
                add_if_within_bounds(point_queue, n + np.array([0, 1]))
                add_if_within_bounds(point_queue, n + np.array([0, -1]))
        # idx+= 1
        # print(idx, len(point_queue))
        # Also need to remove the lines
    coloring += 1
    print(coloring)

# plt.imshow(image.astype(np.int16))

image = np.transpose(image, (1, 0, 2))
img = Image.fromarray(image.astype(np.int8), 'RGB')
img.save('test.png')
