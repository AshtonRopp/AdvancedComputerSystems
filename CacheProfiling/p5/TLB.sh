#!/bin/bash

# Number of trials to run
num_trials=100

# Compile the C++ program
g++ -o TLB.exe TLB_5.cpp

# CSV file to store the results
csv_file="results.csv"

rm $csv_file

# Check if the CSV file exists; if not, add the header
if [ ! -f $csv_file ]; then
    echo "time,tlb_misses" > $csv_file
fi

# Variables to hold cumulative time and TLB misses for averaging
total_time=0
total_tlb_misses=0

# Perform multiple trials
for (( i=1; i<=num_trials; i++ ))
do
    echo "Running trial $i..."

    # Run the C++ program and capture the time output
    runtime=$(./TLB.exe)

    # Use perf to get the TLB misses
    perf_output=$(perf2 stat -e dTLB-load-misses,dTLB-store-misses ./TLB.exe 2>&1)

    # Extract the TLB load and store misses from the perf output
    tlb_load_misses=$(echo "$perf_output" | grep 'dTLB-load-misses' | awk '{print $1}' | tr -d ',')
    tlb_store_misses=$(echo "$perf_output" | grep 'dTLB-store-misses' | awk '{print $1}' | tr -d ',')

    # Check if the extracted values are empty or invalid
    if [ -z "$tlb_load_misses" ] || [ -z "$tlb_store_misses" ]; then
        echo "Error: Failed to extract TLB misses from perf output."
    fi

    # Calculate the total TLB misses (load + store) using bc for large numbers
    tlb_misses=$(echo "$tlb_load_misses + $tlb_store_misses" | bc)

    # Add the current trial's time and TLB misses to the total
    total_time=$(echo "$total_time + $runtime" | bc)
    total_tlb_misses=$(echo "$total_tlb_misses + $tlb_misses" | bc)

    # Append the trial's results to the CSV file
    echo "$runtime,$tlb_misses" >> $csv_file
done

# Calculate average time and TLB misses
avg_time=$(echo "scale=6; $total_time / $num_trials" | bc)
avg_tlb_misses=$(echo "scale=0; $total_tlb_misses / $num_trials" | bc)

echo "Average Time: $avg_time, Average TLB Misses: $avg_tlb_misses"

# Run the Python script to generate the graph
python3 TLB.py
