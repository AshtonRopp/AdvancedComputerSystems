import matplotlib.pyplot as plt
import pandas as pd
from scipy.optimize import curve_fit
import numpy as np

# Load the CSV data
data = pd.read_csv('results.csv')

# Convert TLB misses from string to numeric for plotting
data['tlb_misses'] = pd.to_numeric(data['tlb_misses'])

# Define a polynomial function (adjust the degree as needed)
def func(x, a, b, c):
    return a*x**2 + b*x + c

# Fit the curve to the data
popt, _ = curve_fit(func, data['tlb_misses'], data['time'])

# Create the fitted curve
x_fit = np.linspace(min(data['tlb_misses']), max(data['tlb_misses']), 100)
y_fit = func(x_fit, *popt)

# Plot TLB misses on the X-axis and time on the Y-axis
plt.scatter(data['tlb_misses'], data['time'], marker='o')

# Plot the line of best fit
plt.plot(x_fit, y_fit, color='red')

# Add title and labels
plt.title('TLB Misses vs Time (Non-Linear Fit)')
plt.xlabel('TLB Misses')
plt.ylabel('Time (seconds)')
plt.grid(True)

# Save the figure as a PNG file
plt.savefig('tlb_vs_time_nonlinear.png')

# Close the plot to avoid it staying open
plt.close()

print("Graph saved as 'tlb_vs_time_nonlinear.png'.")