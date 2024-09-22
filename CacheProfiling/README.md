# Cache Memory Profiling
## System Research
Using the hwloc Linux tool, I can reveal the cache structure shown below:
<p align="center">
  <img src="images/hwloc.png" alt="hwloc tool output" />
</p>


From this, we can see that each core in my computer has its own L1 and L2 cache.  Further information, such as associativity, can be gathered from running "getconf -a | grep CACHE".

<p align="center">
  <img src="images/getconf.png" alt="hwloc tool output" />
</p>

We can also use CPU-Z to confirm this on the Windows side.
<p align="center">
  <img src="images/cpu_z.png" alt="cpu_z tool output" />
</p>

## Assignment #1: Read/Write Latency with Queue Length = 0
### Cache
Questions: is this ensuring 0 queue?
Notes: Add info about cache prefetching
Todo convert to array structures and observe changes --> these  are all stored next to each other
### Main Memory

## Assignment #1: Memory Bandwidth Across Varied Data Granularity
Notes: Multithreading
