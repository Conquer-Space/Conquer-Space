with open("satellites.txt", "r") as file:
    lines = file.readlines()
    for k in range(int(len(lines)/3)):
        if "NAVSTAR" in lines[k]:
            print(lines[k].strip())
            # Also add the next 2 lines
            print(lines[k + 1].strip())
            print(lines[k + 2].strip())
