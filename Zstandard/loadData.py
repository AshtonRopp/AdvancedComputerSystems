from datasets import load_dataset
import json
import numpy as np

imdb = load_dataset("imdb")
raw_reviews = [row["text"] for row in imdb["train"]]  # List of raw strings
with open("ml_data/imdb_raw.txt", "w") as f:
    f.write("\n".join(raw_reviews))


wikitext = load_dataset("wikitext", "wikitext-2-raw-v1")
raw_text = [row["text"] for row in wikitext["train"]]
with open("ml_data/wikitext2_raw.txt", "w") as f:
    f.write("\n".join(raw_text))

# Load CIFAR-10 dataset
cifar10 = load_dataset("cifar10")

# Open binary file for train split
with open("ml_data/cifar10_train.bin", "wb") as f:
    for example in cifar10["train"]:
        # Convert image and label to binary
        img_bytes = np.array(example["img"]).tobytes()
        label_bytes = example["label"].to_bytes(1, 'big')  # 1 byte for the label
        # Write label followed by image bytes
        f.write(label_bytes + img_bytes)

# Load Fashion-MNIST dataset
fashion_mnist = load_dataset("fashion_mnist")

# Open binary file for train split
with open("ml_data/fashion_mnist_train.bin", "wb") as f:
    for example in fashion_mnist["train"]:
        # Convert image and label to binary
        img_bytes = np.array(example["image"]).tobytes()  # Flatten image to bytes
        label_bytes = example["label"].to_bytes(1, 'big')  # 1 byte for the label
        # Write label followed by image bytes
        f.write(label_bytes + img_bytes)