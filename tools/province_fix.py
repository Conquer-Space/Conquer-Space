import sys

def parse_def_line(line):
    province, r, g, b, country = line.split(",")
    rgb = (int(r), int(g), int(b))
    return rgb, province, country

def main(def_name):
    with open(def_name, "r") as definition_file:
        def_data = definition_file.read()
        def_data = [parse_def_line(line) for line in def_data.splitlines()]
        def_data.sort(key=lambda k: k[1])
        last_item = [0, 0, 0]
        triggered_last = False
        definition_list = []
        current_list = set()
        ignored_count = 0
        same_name_count = 1
        for data in def_data:
            if data[2] == "":
                # print(f"Province {data[1]} has empty country!")
                ignored_count += 1
                continue
            if data[1] == last_item[1]:
                # then it's an issue lmao
                # then set the new name?
                new_name = data[1] + "_" + data[2]
                if new_name in current_list:
                    same_name_count += 1
                    new_name = new_name + "_" + str(same_name_count)

                # Therefore also link the previous element to be something else as well
                if not triggered_last:
                    definition_list[-1]= (last_item[1] + "_" + last_item[2], last_item[0][0], last_item[0][1], last_item[0][2], last_item[2])
                    current_list.add(last_item[1] + "_" + last_item[2])
                    # check if they're the same?
                    if last_item[1] + "_" + last_item[2] == new_name:
                        same_name_count += 1
                        new_name = new_name + "_" + str(same_name_count)

                # Replace the last item
                triggered_last = True
                data = (new_name, data[0][0], data[0][1], data[0][2], data[2])
                definition_list.append(data)
                current_list.add(new_name)
                print(new_name)
            else:
                last_item = data
                same_name_count = 1
                triggered_last = False
                definition_list.append((data[1], data[0][0], data[0][1], data[0][2], data[2]))
                current_list = set()

        # Make sure they're equal
        print(len(definition_list), ignored_count, ignored_count + len(definition_list), len(def_data))
        # Output the csv file or something
        # Convert to the output
    with open(def_name + "_update", "w") as out_file:
        for data in definition_list:
            out_file.write(f"{data[0]},{data[1]},{data[2]},{data[3]},{data[4]}\n")

# Fix the province map
if __name__ == "__main__":
    if len(sys.argv) > 1:
        main(sys.argv[1])
    else:
        print(f"Usage: python {sys.argv[0]} [province definitions csv file name]")
