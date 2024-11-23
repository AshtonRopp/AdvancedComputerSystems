import subprocess
import matplotlib.pyplot as plt
import numpy as np

def getLines(capturedText):
    split = capturedText.split("\n")

    # print(split)

    ret = []

    for i in range(len(split)):
        if split[i].find('#') != -1:
            ret.append(split[i-1])

            j = i - 1
            while j > 0:
                if split[j].count(',') == 2:
                    ret[-1] = split[j]
                    break
                j -= 1

    return ret


def getVals(lines):
    compFactors = []
    compSpeed = []
    decompSpeed = []

    for line in lines:
        pos1 = line.find('(') + 1
        pos2 = line.find(')')
        compFactors.append(float(line[pos1:pos2]))

        split = line.split(",")

        if(len(split)) != 3:
            print("Incorrect parsing occurred")
            exit()

        compSpeed.append(float(split[1][:-5]))
        decompSpeed.append(float(split[2][:-5]))

    return np.array(compFactors), np.array(compSpeed), np.array(decompSpeed)


# Define directories and test files
cifar_dir = "ml_data/cifar10_train.bin"
fashion_dir = "ml_data/fashion_mnist_train.bin"
imdb_dir = "ml_data/imdb_raw.txt"
wikitext_dir = "ml_data/wikitext2_raw.txt"

dataSets = [cifar_dir, fashion_dir, imdb_dir, wikitext_dir]
dataSetNames = ["CIFAR-10", "Fashion MNIST", "IMDB Reviews", "WikiText2"]

num_tests = 3
start_i = 1
end_i = 12

start = "-b" + str(start_i)
end = "-e" + str(end_i)

if __name__ == "__main__":
    all_compFactors = []
    all_compSpeed = []
    all_decompSpeed = []

    for dataSet in dataSets:
        cmd = ["zstd", start, end, "-r", dataSet]
        compFactors = np.zeros(end_i - start_i + 1)
        compSpeed = np.zeros(end_i - start_i + 1)
        decompSpeed = np.zeros(end_i - start_i + 1)

        for t in range(num_tests):
            result = subprocess.run(cmd, check=True, capture_output=True, text=True)
            lines = getLines(result.stderr)
            vals = getVals(lines)

            compFactors += vals[0] / num_tests
            compSpeed += vals[1] / num_tests
            decompSpeed += vals[2] / num_tests

        all_compFactors.append(compFactors)
        all_compSpeed.append(compSpeed)
        all_decompSpeed.append(decompSpeed)

    x_range = range(start_i, end_i + 1)

    # Compression Factors Graph
    plt.figure(figsize=(10, 6))
    for i, compFactors in enumerate(all_compFactors):
        plt.plot(x_range, compFactors, label=dataSetNames[i])
    plt.title("Compression Factors")
    plt.xlabel("Compression Level")
    plt.ylabel("Compression Ratio")
    plt.legend()
    plt.grid(True)
    plt.savefig("compression_factors.png")

    # Compression Speed Graph
    plt.figure(figsize=(10, 6))
    for i, compSpeed in enumerate(all_compSpeed):
        plt.plot(x_range, compSpeed, label=dataSetNames[i])
    plt.title("Compression Speed")
    plt.xlabel("Compression Level")
    plt.ylabel("Speed (MB/s)")
    plt.legend()
    plt.grid(True)
    plt.savefig("compression_speed.png")

    # Decompression Speed Graph
    plt.figure(figsize=(10, 6))
    for i, decompSpeed in enumerate(all_decompSpeed):
        plt.plot(x_range, decompSpeed, label=dataSetNames[i])
    plt.title("Decompression Speed")
    plt.xlabel("Compression Level")
    plt.ylabel("Speed (MB/s)")
    plt.legend()
    plt.grid(True)
    plt.savefig("decompression_speed.png")

    print("Graphs saved as PNG files.")