import pandas as pd
import matplotlib.pyplot as plt
import sys

if __name__ == "__main__":
    for i in sys.argv:
        print(i)

    # Check for proper number of input args
    if len(sys.argv) != 2:
        print("Incorrect CLI arguments!")
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
        ax1.legend(loc='lower right')
        ax1.set_title('Read and Write Speed vs ' + x_header)

        # Plot on the second subplot (Latency)
        ax2.set_xlabel(x_header)
        ax2.set_ylabel('Latency (ms)')
        ax2.plot(x, y1_2, label='Read Latency', color='tab:blue', marker='o')
        ax2.plot(x, y2_2, label='Write Latency', color='tab:red', marker='o')
        ax2.tick_params(axis='y')
        ax2.legend(loc='lower right')
        ax2.set_title('Latency vs ' + x_header)

        # Adjust layout to avoid overlapping
        plt.tight_layout()

        # Save the plot as an image file (e.g., PNG format)
        plt.savefig('plot.png', format='png', dpi=300, bbox_inches='tight')

        # Optionally, you can clear the figure
        plt.close(fig)

    elif sys.argv[1].strip() == "combine_both":
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

        # Set a common title for the figure
        fig.suptitle(x_header + ' vs Speed and Latency Metrics')

        # Add legends below each subplot
        ax1.legend(loc='upper left')
        ax2.legend(loc='upper left')

        # Adjust layout to prevent overlapping
        plt.tight_layout(rect=[0, 0, 1, 0.95])

        # Save the plot as an image file (e.g., PNG format)
        plt.savefig('plot.png', format='png', dpi=300, bbox_inches='tight')

        # Optionally, you can clear the figure
        plt.close(fig)
