# Used to plot the csv logs of market data
import pandas as pd
import matplotlib.pyplot as plt
import sys

def main():
    if len(sys.argv) == 1:
        print("Specify csv file with the arguments!")
        return
    file_name = sys.argv[1]
    good_name = sys.argv[2]
    csv_output = pd.read_csv(file_name)

    good_list = [s[0:-7] for s in csv_output.columns[6::10]]
    good_idx = good_list.index(good_name)

    for i in range(10):
        col_name = csv_output.columns[6 + (good_idx * 10) + i]
        plt.subplot(5, 2, i + 1)
        plt.title(col_name)
        if i != 4 or i != 9:
            plt.xticks([])
        plt.plot(csv_output["Date"], csv_output[col_name])

    plt.show()

if __name__ == "__main__":
    main()
