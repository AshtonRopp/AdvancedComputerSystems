from datasets import load_dataset
import os
import subprocess
import shutil
import time

# Step 1: Download a large dataset
dataset_name = "wikipedia"  # Switch to a larger dataset
subset_name = "20220301.simple"  # Use the simplified English Wikipedia subset
output_dir = "zstd_training_data"  # Directory to save raw files and dictionary
os.makedirs(output_dir, exist_ok=True)

# Load the dataset
print("Downloading dataset...")
dataset = load_dataset(dataset_name, subset_name, split="train")
print("Finished downloading ...")

if not os.path.exists(output_dir + "/samples"):
    os.mkdir(output_dir + "/samples")

# Extract samples and save them as individual files
sample_files = []
for idx, sample in enumerate(dataset):
    sample_text = sample["text"]  # Assuming the dataset has a 'text' field
    sample_path = output_dir + "/samples/" + f"s{idx}.txt"
    with open(sample_path, "w", encoding="utf-8") as f:
        f.write(sample_text)
    sample_files.append(sample_path)

    # Limiting to first 100 samples for demonstration (adjust as needed)
    if idx >= 99:
        break

print(f"Saved {len(sample_files)} samples for dictionary training.")

# Step 2: Train a zstd dictionary
dictionary_path = os.path.join(output_dir, "zstd_dictionary.dict")

# Train the dictionary using the zstd command-line tool
subprocess.run(
    f"zstd --train zstd_training_data/samples/* -o {dictionary_path}",
    shell=True,
    check=True,
    text=True,
    capture_output=True
)
print(f"zstd dictionary saved to {dictionary_path}")

# Step 3: Test dictionary-based compression
sample_to_compress = sample_files[0]  # Use the first sample for testing
compressed_file_path_dict = sample_to_compress + ".zst.dict"

start_time_dict = time.perf_counter()
subprocess.run(
    f"zstd -D {dictionary_path} {sample_to_compress} -o {compressed_file_path_dict}",
    shell=True,
    check=True,
    text=True,
    capture_output=True
)
time_taken_dict = time.perf_counter() - start_time_dict
print(f"Dictionary-based compressed file saved to {compressed_file_path_dict}")

# Decompress dictionary-based compressed file
decompressed_file_path_dict = compressed_file_path_dict + ".decompressed"
start_time_dict_decompress = time.perf_counter()
subprocess.run(
    f"zstd -D {dictionary_path} -d {compressed_file_path_dict} -o {decompressed_file_path_dict}",
    shell=True,
    check=True,
    text=True,
    capture_output=True
)
time_taken_dict_decompress = time.perf_counter() - start_time_dict_decompress
print(f"Dictionary-based decompressed file saved to {decompressed_file_path_dict}")

# Step 4: Test standard zstd compression (no dictionary)
compressed_file_path_std = sample_to_compress + ".zst.std"

start_time_std = time.perf_counter()
subprocess.run(
    f"zstd {sample_to_compress} -o {compressed_file_path_std}",
    shell=True,
    check=True,
    text=True,
    capture_output=True
)
time_taken_std = time.perf_counter() - start_time_std
print(f"Standard zstd compressed file saved to {compressed_file_path_std}")

# Decompress standard compressed file
decompressed_file_path_std = compressed_file_path_std + ".decompressed"
start_time_std_decompress = time.perf_counter()
subprocess.run(
    f"zstd -d {compressed_file_path_std} -o {decompressed_file_path_std}",
    shell=True,
    check=True,
    text=True,
    capture_output=True
)
time_taken_std_decompress = time.perf_counter() - start_time_std_decompress
print(f"Standard decompressed file saved to {decompressed_file_path_std}")

# Step 5: Print compression and decompression results
original_size = os.path.getsize(sample_to_compress)
compressed_size_dict = os.path.getsize(compressed_file_path_dict)
compressed_size_std = os.path.getsize(compressed_file_path_std)

print(f"Original size: {original_size} bytes")
print(f"Dictionary-based compressed size: {compressed_size_dict} bytes")
print(f"Time taken for dictionary-based compression: {time_taken_dict:.4f} seconds")
print(f"Time taken for dictionary-based decompression: {time_taken_dict_decompress:.4f} seconds")
print(f"Standard zstd compressed size: {compressed_size_std} bytes")
print(f"Time taken for standard compression: {time_taken_std:.4f} seconds")
print(f"Time taken for standard decompression: {time_taken_std_decompress:.4f} seconds")

# Step 6: Clean up the generated directory
if os.path.exists("zstd_training_data/"):
    shutil.rmtree("zstd_training_data/")
