from datasets import load_dataset
import os
import subprocess
import shutil
import time
import matplotlib.pyplot as plt
import random

# Configuration
dataset_name = "mnist"
subset_name = "train"  # You can also use "test" for test data
output_dir = "zstd_training_data"
dictionary_path = os.path.join(output_dir, "zstd_dictionary.dict")
num_trials = 100  # Number of trials to average over

# Ensure output directory exists
os.makedirs(output_dir, exist_ok=True)

# Step 1: Download the dataset
print("Downloading dataset...")
dataset = load_dataset(dataset_name, split=subset_name)
print("Finished downloading.")

# Step 2: Function to save samples as PNG images
def save_samples(output_dir, num_samples):
    samples_dir = os.path.join(output_dir, "samples")
    if os.path.exists(samples_dir):
        shutil.rmtree(samples_dir)
    os.makedirs(samples_dir)

    sample_files = []
    for idx, sample in enumerate(dataset):
        # Convert MNIST images from PIL format to file
        sample_image = sample["image"]
        label = sample["label"]
        sample_path = os.path.join(samples_dir, f"s{idx}_{label}.png")
        sample_image.save(sample_path)
        sample_files.append(sample_path)
        if idx + 1 >= num_samples:
            break
    return sample_files

# Step 3: Standard compression metrics (calculated once)
print("\nCalculating standard compression metrics...")
sample_files = save_samples(output_dir, 100)  # Save 100 samples for standard metrics
sample_to_compress = sample_files[0]

compressed_file_path_std = sample_to_compress + ".zst.std"

start_time_std = time.perf_counter()
subprocess.run(
    f"zstd {sample_to_compress} -o {compressed_file_path_std}",
    shell=True, check=True, text=True, capture_output=True
)
time_taken_std = time.perf_counter() - start_time_std
compressed_size_std = os.path.getsize(compressed_file_path_std)

# Decompression
decompressed_file_path_std = compressed_file_path_std + ".decompressed"
start_time_std_decompress = time.perf_counter()
subprocess.run(
    f"zstd -d {compressed_file_path_std} -o {decompressed_file_path_std}",
    shell=True, check=True, text=True, capture_output=True
)
time_taken_std_decompress = time.perf_counter() - start_time_std_decompress

# Print standard metrics
original_size = os.path.getsize(sample_to_compress)
print(f"\nStandard Compression Metrics:")
print(f"Original size: {original_size} bytes")
print(f"Standard compressed size: {compressed_size_std} bytes")
print(f"Time taken for standard compression: {time_taken_std:.4f} seconds")
print(f"Time taken for standard decompression: {time_taken_std_decompress:.4f} seconds")

# Step 4: Gather metrics for various sample sizes
sample_sizes = [100, 200, 500, 1000, 1500, 2000]  # Adjust sizes as needed
compression_metrics = []

for num_samples in sample_sizes:
    print(f"\nProcessing {num_samples} samples...")

    # Save samples
    sample_files = save_samples(output_dir, num_samples)

    # Train the dictionary once for the current sample size
    start_time_dict_train = time.perf_counter()
    subprocess.run(
        f"zstd --train {output_dir}/samples/* -o {dictionary_path}",
        shell=True, check=True, text=True, capture_output=True
    )
    time_taken_dict_train = time.perf_counter() - start_time_dict_train
    print(f"Dictionary trained in {time_taken_dict_train:.4f} seconds.")

    # Test dictionary-based compression (average over multiple trials)
    compression_ratios = []
    compression_times = []
    decompression_times = []
    for _ in range(num_trials):
        # Randomly select a sample for compression
        sample_to_compress = random.choice(sample_files)
        compressed_file_path_dict = sample_to_compress + ".zst.dict"

        start_time_dict = time.perf_counter()
        subprocess.run(
            f"zstd -D {dictionary_path} {sample_to_compress} -f -o {compressed_file_path_dict}",
            shell=True, check=True, text=True, capture_output=True
        )
        time_taken_dict = time.perf_counter() - start_time_dict
        compression_times.append(time_taken_dict)
        
        compressed_size_dict = os.path.getsize(compressed_file_path_dict)
        # Calculate compression ratio
        compression_ratio = os.path.getsize(sample_to_compress) / compressed_size_dict
        compression_ratios.append(compression_ratio)

        # Test dictionary-based decompression (average over multiple trials)
        decompressed_file_path_dict = compressed_file_path_dict + ".decompressed"
        start_time_dict_decompress = time.perf_counter()
        subprocess.run(
            f"zstd -D {dictionary_path} -d {compressed_file_path_dict} -f -o {decompressed_file_path_dict}",
            shell=True, check=True, text=True, capture_output=True
        )
        time_taken_dict_decompress = time.perf_counter() - start_time_dict_decompress
        decompression_times.append(time_taken_dict_decompress)

    # Calculate averages for compression ratios, compression times, and decompression times
    avg_compression_ratio = sum(compression_ratios) / num_trials
    avg_compression_time = sum(compression_times) / num_trials
    avg_decompression_time = sum(decompression_times) / num_trials

    # Append results
    compression_metrics.append({
        "sample_size": num_samples,
        "avg_compression_ratio": avg_compression_ratio,
        "compression_time": avg_compression_time,
        "decompression_time": avg_decompression_time,
    })

# Step 5: Plot results
sample_sizes = [entry["sample_size"] for entry in compression_metrics]
avg_compression_ratios = [entry["avg_compression_ratio"] for entry in compression_metrics]
compression_times = [entry["compression_time"] for entry in compression_metrics]
decompression_times = [entry["decompression_time"] for entry in compression_metrics]

plt.figure(figsize=(15, 5))

# Compression ratio vs. Dictionary Sample Size
plt.subplot(1, 3, 1)
plt.plot(sample_sizes, avg_compression_ratios, marker="o", label="Compression Ratio")
plt.xlabel("Sample Size")
plt.ylabel("Compression Ratio")
plt.title("Compression Ratio vs. Dictionary Sample Size")
plt.grid(True)
plt.legend(loc='lower right')  # Place legend in the bottom right

# Compression time vs. Dictionary Sample Size
plt.subplot(1, 3, 2)
plt.plot(sample_sizes, compression_times, marker="o", color="g", label="Compression Time")
plt.xlabel("Sample Size")
plt.ylabel("Time (seconds)")
plt.title("Compression Time vs. Dictionary Sample Size")
plt.grid(True)
plt.legend(loc='lower right')  # Place legend in the bottom right

# Decompression time vs. Dictionary Sample Size
plt.subplot(1, 3, 3)
plt.plot(sample_sizes, decompression_times, marker="o", color="r", label="Decompression Time")
plt.xlabel("Sample Size")
plt.ylabel("Time (seconds)")
plt.title("Decompression Time vs. Dictionary Sample Size")
plt.grid(True)
plt.legend(loc='lower right')  # Place legend in the bottom right

plt.tight_layout()
plt.savefig("zstd_metrics_mnist.png")

# Step 6: Clean up the generated directory
if os.path.exists(output_dir):
    shutil.rmtree(output_dir)
