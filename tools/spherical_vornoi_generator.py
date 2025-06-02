# Generates a series of points on a sphere, then generates a vornoi diagram based of it, and outputs it onto an image
import numpy as np
from scipy.spatial import SphericalVoronoi, geometric_slerp
import matplotlib.pyplot as plt
import collections
from PIL import Image
import argparse
import sys
import string
import random

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

def lat_lon_to_image(points: np.ndarray, width: int, height: int):
    x_points = np.round(((points[:,0]) / np.pi + 1) * (width // 2))
    y_points = np.round(((points[:,1]) / (np.pi)) * (height))
    # Would be better if we removed this
    x_points = np.clip(x_points, a_min=0, a_max=width - 1)
    y_points = np.clip(y_points, a_min=0, a_max=height - 1)
    x_points = x_points.astype(np.int16)
    y_points = y_points.astype(np.int16)
    return x_points, y_points

def add_if_within_bounds(queue: collections.deque, point: np.ndarray, width, height) -> None:
    queue.append(np.array([point[0] % width, point[1] % height]))

def is_in_group(output_image, image, point):
    return np.all(image[*point] == np.zeros(3))

def generate_province_segment(size: int = 5):
    return "".join(random.choices(string.ascii_uppercase + string.digits, k=size))

def generate_province_name():
    return f"{generate_province_segment()}-{generate_province_segment()}"

def generate_voronoi_district_map(district_count: int, seed: int, width: int, height: int, output_filename: str, province_filename: str):
    voronoi_points = generate_random_points(district_count, seed)
    random.seed(seed)
    sv = SphericalVoronoi(voronoi_points)
    sv.calculate_areas()
    t_vals = np.linspace(0, 1, 2000)

    output_shape = (width, height)
    output_image = np.zeros(output_shape)
    for region in sv.regions:
        n = len(region)
        for i in range(n):
            start = sv.vertices[region][i]
            end = sv.vertices[region][(i + 1) % n]
            result = geometric_slerp(start, end, t_vals)
            # Now convert all the points to an image
            points = to_latitude_longitude(result)
            x_points, y_points = lat_lon_to_image(points, width, height)

            # TODO: Instead of relying on maxing out the t_vals so that we oversaturate the different borders,
            # we should just lerp the points between each vertex
            # I'm not sure if this is a function or we can have multiple outputs for one input
            output_image[x_points, y_points] = 1

    print("Done initializing regions!")

    # Now floodfill everything
    # I think we should greedly take what is present
    # We can assume that each
    image = np.zeros((*output_shape, 3))
    # We can floodfill starting from the original points
    # Color picking...

    coloring = 1
    voronoi_points = to_latitude_longitude(voronoi_points)
    voronoi_points = np.vstack(lat_lon_to_image(voronoi_points, width, height)).T

    province_names = []
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
                # this helps remove the lines between the various provinces as well
                # this terminates the floodfill on the lines
                if output_image[*n] == 0:
                    add_if_within_bounds(point_queue, n + np.array([1, 0]), width, height)
                    add_if_within_bounds(point_queue, n + np.array([-1, 0]), width, height)
                    add_if_within_bounds(point_queue, n + np.array([0, 1]), width, height)
                    add_if_within_bounds(point_queue, n + np.array([0, -1]), width, height)
            # idx+= 1
            # print(idx, len(point_queue))
        name = generate_province_name()
        province_names.append((name, *district_color))
        print(f"Coloring province {coloring}/{district_count}", end="\r")
        coloring += 1

    # We also need funcitons to generate the names and colors of the individual districts as well
    image = np.transpose(image, (1, 0, 2))
    img = Image.fromarray(image.astype(np.int8), 'RGB')
    img.save(output_filename)
    print(f"Written image to {output_filename}!", end="")
    # Write csv output
    with open(province_filename, "w") as file:
        for prov in province_names:
            file.write(f"{prov[0]},{prov[1]},{prov[2]},{prov[3]},\n")

# Just a simple wrapper to generate the file
if __name__ == "__main__":
    # In the future, we want to be able to take in a heightmap, and generate the voronoi based off that height map
    parser = argparse.ArgumentParser(prog="Voronoi Province Generator")
    parser.add_argument("-n", "--count", help="Number of voronoi cells", required=True, type=int)
    parser.add_argument("-s", "--seed", help="Seed of the voronoi point generator", default=0, type=int)
    parser.add_argument("-o", "--output", help="Output file name of the voronoi image map (normally a png)", default="out.png")
    parser.add_argument("-p", "--province-output", help="Output file of the province colors and randomly generated names", default="out.csv")
    parser.add_argument("-d", "--dimension", help="width,height tuple", default="1024,512")
    arguments = parser.parse_args()

    if len(sys.argv) == 1 or ("help" in arguments):
        parser.print_help()
        exit()

    # Ideally we will do some error detection but
    width, height = arguments.dimension.split(",")
    width = int(width)
    height = int(height)
    generate_voronoi_district_map(arguments.count,
                                    arguments.seed,
                                    width,
                                    height,
                                    arguments.output,
                                    arguments.province_output)
