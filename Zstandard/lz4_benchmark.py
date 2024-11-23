import subprocess
import matplotlib.pyplot as plt
import numpy as np
import os
import time

def get_lz4_speed(file, level, mode):
    """
    Measure the compression or decompression speed of lz4.
    Args:
        file (str): File to compress or decompress.
        level (int): Compression level (1-9 for simplicity; lz4 uses different presets).
        mode (str): 'compress' or 'decompress'.

    Returns:
        float: Speed in MB/s.
    """
    start_time = time.time()
    if mode == "compress":
        compressed_file = file + ".lz4"
        cmd = f"lz4 -{level} -f {file} {compressed_file}"  # Added '-f' for force overwrite
    elif mode == "decompress":
        compressed_file = file + ".lz4"
        decompressed_file = file + ".out"
        cmd = f"lz4 -d -f {compressed_file} {decompressed_file}"  # Added '-f' for force overwrite
    else:
        raise ValueError("Invalid mode")

    # Run the command
    subprocess.run(cmd, shell=True, check=True)

    # Calculate elapsed time
    elapsed_time = time.time() - start_time

    # Determine file size
    if mode == "compress":
        file_size = os.path.getsize(file)
    elif mode == "decompress":
        file_size = os.path.getsize(compressed_file)

    # Clean up temporary files
    if mode == "decompress":
        os.remove(decompressed_file)

    # Return speed in MB/s
    return file_size / (1024 ** 2) / elapsed_time

def get_compression_factor(original_file, compressed_file):
    """
    Calculate compression factor (original size / compressed size).
    Args:
        original_file (str): Original file path.
        compressed_file (str): Compressed file path.

    Returns:
        float: Compression factor.
    """
    original_size = os.path.getsize(original_file)
    compressed_size = os.path.getsize(compressed_file)
    return original_size / compressed_size

# Define directories and test files
cifar_dir = "ml_data/cifar10_train.bin"
fashion_dir = "ml_data/fashion_mnist_train.bin"
imdb_dir = "ml_data/imdb_raw.txt"
wikitext_dir = "ml_data/wikitext2_raw.txt"

dataSets = [cifar_dir, fashion_dir, imdb_dir, wikitext_dir]
dataSetNames = ["CIFAR-10", "Fashion MNIST", "IMDB Reviews", "WikiText2"]

num_tests = 6
start_i = 1
end_i = 9  # Simplified compression levels for lz4

if __name__ == "__main__":
    all_compFactors = []
    all_compSpeed = []
    all_decompSpeed = []

    for dataSet in dataSets:
        compFactors = np.zeros(end_i - start_i + 1)
        compSpeed = np.zeros(end_i - start_i + 1)
        decompSpeed = np.zeros(end_i - start_i + 1)

        for level in range(start_i, end_i + 1):
            for t in range(num_tests):
                # Measure compression speed
                comp_speed = get_lz4_speed(dataSet, level, "compress")
                compSpeed[level - start_i] += comp_speed / num_tests

                # Measure decompression speed
                decomp_speed = get_lz4_speed(dataSet, level, "decompress")
                decompSpeed[level - start_i] += decomp_speed / num_tests

                # Measure compression factor
                lz4_file = dataSet + ".lz4"
                comp_factor = get_compression_factor(dataSet, lz4_file)
                compFactors[level - start_i] += comp_factor / num_tests

            # Clean up compressed file after all tests
            os.remove(dataSet + ".lz4")

        all_compFactors.append(compFactors)
        all_compSpeed.append(compSpeed)
        all_decompSpeed.append(decompSpeed)

    x_range = range(start_i, end_i + 1)

    # Compression Factors Graph
    plt.figure(figsize=(10, 6))
    for i, compFactors in enumerate(all_compFactors):
        plt.plot(x_range, compFactors, label=dataSetNames[i])
    plt.title("Compression Factors (lz4)")
    plt.xlabel("Compression Level")
    plt.ylabel("Compression Ratio")
    plt.legend()
    plt.grid(True)
    plt.savefig("lz4_compression_factors.png")

    # Compression Speed Graph
    plt.figure(figsize=(10, 6))
    for i, compSpeed in enumerate(all_compSpeed):
        plt.plot(x_range, compSpeed, label=dataSetNames[i])
    plt.title("Compression Speed (lz4)")
    plt.xlabel("Compression Level")
    plt.ylabel("Speed (MB/s)")
    plt.legend()
    plt.grid(True)
    plt.savefig("lz4_compression_speed.png")

    # Decompression Speed Graph
    plt.figure(figsize=(10, 6))
    for i, decompSpeed in enumerate(all_decompSpeed):
        plt.plot(x_range, decompSpeed, label=dataSetNames[i])
    plt.title("Decompression Speed (lz4)")
    plt.xlabel("Compression Level")
    plt.ylabel("Speed (MB/s)")
    plt.legend()
    plt.grid(True)
    plt.savefig("lz4_decompression_speed.png")

    print("Graphs saved as PNG files.")
