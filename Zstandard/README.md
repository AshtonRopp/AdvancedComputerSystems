## Zstandard Compression Testing

## Background Research
The following section will summarize my research into the ZStandard (zstd) algorithm. For more thorough documentation, refer to the project's compression format guide found [here](https://github.com/facebook/zstd/blob/dev/doc/zstd_compression_format.md).

### Frames
zstd compresses data into what is referred to as "frames". The format of these is shown below, taken from the Zstandard GitHub page.

| `Magic_Number` | `Frame_Header` |`Data_Block`| [More data blocks] | [`Content_Checksum`] |
|:--------------:|:--------------:|:----------:| ------------------ |:--------------------:|
|  4 bytes       |  2-14 bytes    |  n bytes   |                    |     0-4 bytes        |

- `Magic_Number`: Identifier which would be unlikely to start text file
- `Frame_Header`: Specifies frame attributes
- `Data_Block`: Data storage
- `Content_Checksum`: Error checking data

For a complete overview of frames, refer to [this](https://github.com/facebook/zstd/blob/dev/doc/zstd_compression_format.md#zstandard-frames) page.

### Window_Size
This is the minimum buffer size that the program will require for compression or decompression. 8 MB is the recommended max for this value. Generally, increasing `Window_Size` increases compression ratio but makes the process more memory intensive.

### Blocks
zstd stores data as "blocks". A block must include 3 bytes for its header. Block size is limited by `Block_Maximum_Size`, which is defined as the smaller value between `Window_Size` and 128 KB. Each frame has a set `Block_Maximum_Size`.

### Compression/Decompression
Compressed blocks contain a literals and sequences section. Literals can be stored regularly or compressed with Huffman prefixing. If using Huffman, there will be a tree included for decoding, or the previous dictionary will be used. The sequences section then consists of compressed references to the literals section.

To decompress these blocks, we must process both the literals and sequences sections.

### Finite State Entropy Encoding (FSE)
During my explanation, I omitted many symbols and specifiers for reader clarity. However, there are many pieces of data or tags we add to the compressed format for optimization purposes. One of the defining features of zstd is that it employs a second algorithm in addition to Huffman coding on the added tags. This format is FSE, which is described below.

1. **Build a Probability Model**:
   - Analyze input data to calculate symbol frequencies.
   - Normalize probabilities to fit a power-of-two space.

2. **Encoding**:
   - Use a finite-state machine (FSM) to encode symbols compactly.
   - Frequent symbols use fewer bits; rare symbols use more.
   - Output a tightly packed bitstream.

3. **Decoding**:
   - Start from the final encoding state.
   - Reverse the FSM process to reconstruct the original data.

An in-depth explanation of the algorithm can be found [here](https://github.com/facebook/zstd/blob/dev/doc/zstd_compression_format.md#fse).

## Experiments
- Trained vs non-trained
- Comparison to other algorithms
- Dictionary benchmark?
- stream options
- threads

# Resources:
https://fuchsia.googlesource.com/third_party/zstd/+/refs/tags/v1.3.7/programs/README.md
https://raw.githack.com/facebook/zstd/release/doc/zstd_manual.html
https://github.com/facebook/zstd/blob/dev/programs/zstd.1.md