import pandas as pd
import matplotlib.pyplot as plt
import sys

if __name__ == "__main__":

    # Check for proper number of input args
    if len(sys.argv) != 2:
        print("Incorrect CLI arguments! See \"Getting Started\" in the project README.")
        exit()

    # Load the CSV file
    data = pd.read_csv('data.csv')

    # Extract data from the DataFrame
    x_header = data.keys()[0]
    x = data[x_header]
    y1 = data['Read Speed']
    y2 = data['Write Speed']
    y1_2 = data['Read Latency']/1e6  # Convert to ms from ns
    y2_2 = data['Write Latency']/1e6 # Convert to ms from ns

    if sys.argv[1].strip() == "plot_both":
        # Create a figure and two subplots horizontally
        fig, (ax1, ax2) = plt.subplots(1, 2, figsize=(12, 5))

        # Plot on the first subplot (Read and Write Speed)
        ax1.set_xlabel(x_header)
        ax1.set_ylabel('Read and Write Speed (MiB/s)')
        ax1.plot(x, y1, label='Read Speed', color='tab:blue', marker='o')
        ax1.plot(x, y2, label='Write Speed', color='tab:red',  marker='o')
        ax1.tick_params(axis='y')
        ax1.set_title('Read and Write Speed vs ' + x_header)

        # Move legend below the first subplot
        ax1.legend(loc='upper center', bbox_to_anchor=(0.5, -0.15), ncol=2)

        # Plot on the second subplot (Latency)
        ax2.set_xlabel(x_header)
        ax2.set_ylabel('Latency (ms)')
        ax2.plot(x, y1_2, label='Read Latency', color='tab:blue', marker='o')
        ax2.plot(x, y2_2, label='Write Latency', color='tab:red', marker='o')
        ax2.tick_params(axis='y')
        ax2.set_title('Latency vs ' + x_header)

        # Move legend below the second subplot
        ax2.legend(loc='upper center', bbox_to_anchor=(0.5, -0.15), ncol=2)

        # Adjust layout to avoid overlapping
        plt.tight_layout()

        # Save the plot as an image file (e.g., PNG format)
        plt.savefig('plot.png', format='png', dpi=300, bbox_inches='tight')

        # Optionally, you can clear the figure
        plt.close(fig)

    elif sys.argv[1].strip() == "combine_rw":
        # Create a figure with two subplots (1 row, 2 columns)
        fig, (ax1, ax2) = plt.subplots(1, 2, figsize=(12, 6))

        # Plot the first subplot (Read and Write Speed)
        ax1.set_xlabel(x_header)
        ax1.set_ylabel('Read and Write Speed (MiB/s)')
        ax1.plot(x, (y1 + y2), label='Read/Write Speed', color='tab:blue', marker='o')
        ax1.tick_params(axis='y')
        ax1.set_title('Read/Write Speed')

        # Plot the second subplot (Latency)
        ax2.set_xlabel(x_header)
        ax2.set_ylabel('Latency (ms)')
        ax2.plot(x, (y1_2 + y2_2), label='Read/Write Latency', color='tab:red', marker='o')
        ax2.tick_params(axis='y')
        ax2.set_title('Read/Write Latency')

        # Add legends at the center of each subplot
        ax1.legend(loc='upper center', bbox_to_anchor=(0.5, -0.15), ncol=1)
        ax2.legend(loc='upper center', bbox_to_anchor=(0.5, -0.15), ncol=1)

        # Adjust layout to prevent overlapping
        plt.tight_layout(rect=[0, 0, 1, 0.85])  # Adjust the rect to provide space for legends

        # Save the plot as an image file (e.g., PNG format)
        plt.savefig('plot.png', format='png', dpi=300, bbox_inches='tight')

        # Optionally, you can clear the figure
        plt.close(fig)
