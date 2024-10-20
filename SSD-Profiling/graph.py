import pandas as pd
import matplotlib.pyplot as plt
import sys

if __name__ == "__main__":
    for i in sys.argv:
        print(i)

    # Check for proper number of input args
    if len(sys.argv) != 3:
        print("Please input a graph type! Valid options are:")
        print("plot_both: Plots both speed, IOPs for read and write --> 4 lines")
        print("plot_average: averages read and write together --> 2 lines")
        print("Second CLI arg is header of x data!")
        exit()
    
    # Load the CSV file
    data = pd.read_csv('data.csv')

    # Extract data from the DataFrame
    x_header = sys.argv[2].strip()
    x = data[x_header]
    y1 = data['Read Speed']
    y2 = data['Write Speed']
    y1_2 = data['Write IOPs']
    y2_2 = data['Read IOPs']

    print(y1+y2, y1_2+y2_2)

  

    if sys.argv[1].strip() == "plot_both":
        # Create a figure and axis
        fig, ax1 = plt.subplots()

        # Plot the first Y-axis data
        ax1.set_xlabel(x_header)
        ax1.set_ylabel('Read and Write Speed (MiB/s)')
        ax1.plot(x, y1, label='Read Speed', color='tab:blue', marker='o')
        ax1.plot(x, y2, label='Write Speed', color='tab:orange', marker='o')
        # ax1.tick_params(axis='y', )

        # Create a second Y-axis
        ax2 = ax1.twinx()
        ax2.set_ylabel('IOPs')
        ax2.plot(x, y1_2, label='Write IOPs', color='tab:red', linestyle='--', marker='s')
        ax2.plot(x, y2_2, label='Read IOPs', color='tab:green', linestyle='--', marker='s')
        # ax2.tick_params(axis='y', labelcolor='tab:blk')

        # Add titles
        plt.title(x_header + ' vs Speed Metrics')

        # Add legends below the figure
        fig.legend(loc='lower center', bbox_to_anchor=(0.5, -0.1), ncol=2)

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

        # Plot the second subplot (IOPs)
        ax2.set_xlabel(x_header)
        ax2.set_ylabel('IOPs')
        ax2.plot(x, (y1_2 + y2_2), label='Read/Write IOPs', color='tab:red', marker='o')
        ax2.tick_params(axis='y')
        ax2.set_title('Read/Write IOPs')

        # Set a common title for the figure
        fig.suptitle(x_header + ' vs Speed and IOPs Metrics')

        # Add legends below each subplot
        ax1.legend(loc='upper left')
        ax2.legend(loc='upper left')

        # Adjust layout to prevent overlapping
        plt.tight_layout(rect=[0, 0, 1, 0.95])

        # Save the plot as an image file (e.g., PNG format)
        plt.savefig('plot.png', format='png', dpi=300, bbox_inches='tight')

        # Optionally, you can clear the figure
        plt.close(fig)

