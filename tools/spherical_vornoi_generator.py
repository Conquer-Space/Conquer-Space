# Generates a series of points on a sphere, then generates a vornoi diagram based of it, and outputs it onto an image
import numpy as np

def generate_random_points(seed: int, count: int):
    """
    Generate a series of latitude and longitude points
    https://mathworld.wolfram.com/SpherePointPicking.html
    """
    np.random.seed(seed)
    points = np.random.rand(count, 2)
    points[:,0] = 2 * np.pi * points[:,0]
    points[:,1] = np.arccos(2 * points[:,1] - 1)
    return points

# https://www.math.kent.edu/~zheng/papers/voronoi_paper_updated.pdf
def HandleCircleEvent(p_i, p_j, p_k):
    ...