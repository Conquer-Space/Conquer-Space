import cv2
import numpy as np
import sys

def parse_def_line(line):
    province, r, g, b, country = line.split(",")
    rgb = (int(r), int(g), int(b))
    return rgb, province, country

def main(image_name, def_name):
    img = cv2.imread(image_name, cv2.IMREAD_UNCHANGED)[...,::-1]
    img = np.asarray(img)
    # Make a table or matrix or something
    color_dict = {}

    print("Image size", img.shape)
    for y in range(img.shape[1]):
        for x in range(img.shape[0]):
            if img[x][y][0] < 255:
                continue
            color_here = tuple(img[x][y])
            color_ahead = tuple(img[(x + 1) % img.shape[0]][y])
            if color_here[1:] not in color_dict:
                color_dict[color_here[1:]] = set()
            if color_ahead[1:] not in color_dict:
                color_dict[color_ahead[1:]] = set()
            if color_here != color_ahead and color_ahead[0] == 255:
                # Then there is an edge between the two?
                color_dict[color_ahead[1:]].add(color_here[1:])
                color_dict[color_here[1:]].add(color_ahead[1:])
            if y >= img.shape[1] - 1:
                continue
            color_below = tuple(img[x][(y + 1) % img.shape[1]])
            if color_below[1:] not in color_dict:
                color_dict[color_below[1:]] = set()
            if color_here != color_below and color_below[0] == 255:
                # Then there is an edge between the two?
                color_dict[color_below[1:]].add(color_here[1:])
                color_dict[color_here[1:]].add(color_below[1:])
    print(color_dict)
    with open(def_name, "r") as definition_file:
        def_data = definition_file.read()
        def_data = def_data.splitlines()
        def_data = [parse_def_line(line) for line in def_data]
        def_data = { rgb : province for rgb, province, _ in def_data }
    print(def_data)
    # Now make the dicts
    connection_map = {}
    for province, neighbors in color_dict.items():
        if province in def_data:
            connection_map[def_data[province]] = [ def_data[neigh] for neigh in neighbors if neigh in def_data]
            for neigh in neighbors:
                if neigh in def_data:
                    print(def_data[neigh], end=",")
    print(connection_map)

if __name__ == "__main__":
    if len(sys.argv) > 2:
        main(sys.argv[1], sys.argv[2])
    else:
        print(f"Usage: python {sys.argv[0]} [map image file name] [province definitions csv file name]")
