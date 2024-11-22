import os
import subprocess
import random
import time
from pathlib import Path
import shutil
import matplotlib.pyplot as plt

# Define directories and test files
base_dir = Path("test_data")
compression_dir = base_dir / "compression"
ml_dir = base_dir / "ml"
logs_dir = base_dir / "logs"
results_dir = base_dir / "results"

# Clean up test data directory before each run
def cleanup():
    print("Cleaning up test_data directory...")
    if base_dir.exists():
        shutil.rmtree(base_dir)
    base_dir.mkdir(parents=True, exist_ok=True)
    print("Cleanup complete.")

# Function to execute system commands
def run_command(command):
    try:
        print(f"Running: {' '.join(command)}")
        result = subprocess.run(command, check=True, capture_output=True, text=True)
        return result.stdout
    except subprocess.CalledProcessError as e:
        print(f"Error: {e.stderr}")
        return None

# Generate test files
def generate_test_files():
    print("Generating test files...")
    # Ensure necessary directories exist
    for directory in [compression_dir, ml_dir, logs_dir, results_dir]:
        directory.mkdir(parents=True, exist_ok=True)  # This will create any missing directories

    # Random binary file
    with open(compression_dir / "random_file.bin", "wb") as f:
        f.write(os.urandom(500 * 1024 * 1024))  # 500 MB

    # Mock ML dataset (CSV)
    with open(ml_dir / "dataset.csv", "w") as f:
        f.write("name,age,gender,score\n")
        for _ in range(1000000):
            f.write(f"John Doe,{random.randint(10, 90)},M,{random.randint(0, 100)}\n")

    # Application log
    with open(logs_dir / "application.log", "w") as f:
        for i in range(1, 1000000):
            f.write(f"Log entry {i}: {time.ctime()}\n")
    print("Test files generated.")

# Function to compress a file
def compress_file(input_file, output_file, level):
    # Ensure the results directory exists
    results_dir.mkdir(parents=True, exist_ok=True)  # Create the results directory if missing
    command = ["zstd", f"-{level}", str(input_file), "-o", str(output_file)]
    run_command(command)

# Function to decompress a file
def decompress_file(compressed_file, output_file):
    # Ensure the results directory exists
    results_dir.mkdir(parents=True, exist_ok=True)  # Create the results directory if missing
    command = ["zstd", "-d", str(compressed_file), "-o", str(output_file)]
    run_command(command)

# Benchmark compression and decompression time and speed
def benchmark_compression(input_file, level):
    compressed_file = results_dir / f"{input_file.stem}_level{level}.zst"
    decompressed_file = results_dir / f"{input_file.stem}_level{level}_decompressed.bin"

    print(f"Benchmarking compression level {level}...")

    # Measure compression time
    start_time = time.time()
    compress_file(input_file, compressed_file, level)
    compression_time = time.time() - start_time

    # Measure decompression time
    start_time = time.time()
    decompress_file(compressed_file, decompressed_file)
    decompression_time = time.time() - start_time

    # Calculate speeds
    file_size = input_file.stat().st_size  # Original file size
    compressed_size = compressed_file.stat().st_size  # Compressed file size
    compression_speed = file_size / compression_time if compression_time > 0 else 0  # In bytes per second
    decompression_speed = compressed_size / decompression_time if decompression_time > 0 else 0  # In bytes per second

    print(f"Compression level {level} took {compression_time:.2f} seconds, Compression Speed: {compression_speed / 1e6:.2f} MB/s.")
    print(f"Decompression level {level} took {decompression_time:.2f} seconds, Decompression Speed: {decompression_speed / 1e6:.2f} MB/s.")

    return compression_speed, decompression_speed

# Main testing function
def main():
    cleanup()

    if not shutil.which("zstd"):
        print("Error: Zstandard (zstd) is not installed. Install it first.")
        return

    generate_test_files()

    # Test file to use for benchmarking
    random_file = compression_dir / "random_file.bin"

    # Test 4: Benchmarking
    print("Benchmarking performance...")
    compression_speeds = []
    decompression_speeds = []
    for level in range(1, 6):
        compression_speed, decompression_speed = benchmark_compression(random_file, level)
        compression_speeds.append(compression_speed / 1e6)  # Convert to MB/s
        decompression_speeds.append(decompression_speed / 1e6)  # Convert to MB/s

    # Plotting the results for compression and decompression speeds
    plt.figure(figsize=(10, 6))
    plt.plot(range(1, 6), compression_speeds, label='Compression Speed (MB/s)', marker='o')
    plt.plot(range(1, 6), decompression_speeds, label='Decompression Speed (MB/s)', marker='o', color='red')
    plt.xlabel('Compression Level')
    plt.ylabel('Speed (MB/s)')
    plt.title('Compression and Decompression Speed vs Compression Level')
    plt.legend()

    # Save the plot
    plt.tight_layout()
    plt.savefig(results_dir / 'compression_decompression_benchmark.png')
    print(f"Compression and decompression speed graph saved at {results_dir / 'compression_decompression_benchmark.png'}")

    print("All tests completed successfully.")

if __name__ == "__main__":
    main()
