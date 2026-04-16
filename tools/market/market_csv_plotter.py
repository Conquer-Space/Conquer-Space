# Used to plot the csv logs of market data
import pandas as pd
import matplotlib.pyplot as plt
import sys

def main():
    if len(sys.argv) == 1:
        print("Specify csv file with the arguments!")
        return
    file_names = sys.argv[1:]

    csv_outputs = [pd.read_csv(file_name) for file_name in file_names]

    good_list = [s[0:-7] for s in csv_outputs[0].columns[6::10]]
    print(good_list)
    good_name = input("Input good name:")

    while good_name:
        if good_name not in good_list:
            continue

        good_idx = good_list.index(good_name)
        plt.figure()
        for i in range(10):
            col_name = csv_outputs[0].columns[6 + (good_idx * 10) + i]
            plt.subplot(5, 2, i + 1)
            plt.title(col_name)
            if i != 4 or i != 9:
                plt.xticks([])
            for csv_output in csv_outputs:
                plt.plot(csv_output["Date"], csv_output[col_name])
        plt.show(block=False)

        good_name = input("Input good name:")

if __name__ == "__main__":
    main()
