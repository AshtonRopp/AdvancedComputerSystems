#include <iostream>
#include <chrono>
#include <vector>

using namespace std;
using namespace chrono;

int main() {
    const int N = 10000000; // Size of the array
    vector<int> a(N, 1), b(N, 2), c(N); // Large arrays for multiplication

    // Start timing
    auto start = high_resolution_clock::now();

    // Perform the multiplication
    for (int i = 0; i < N; ++i) {
        c[i] = a[i] * b[i];
    }

    // Stop timing
    auto end = high_resolution_clock::now();
    duration<double> elapsed = end - start;

    // Output the time to standard output
    cout << elapsed.count() << endl;

    return 0;
}
