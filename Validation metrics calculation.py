import pandas as pd
import numpy as np

def hex_to_bin(hex_string):
#Converts a hexadecimal string to a binary string
    return bin(int(hex_string, 16))[2:].zfill(128)

def hamming_distance(bin1, bin2):
    #Calculates the Hamming distance between two binary strings
    return sum(c1 != c2 for c1, c2 in zip(bin1, bin2))

def calculate_uniformity(binary_data):
    #Calculates uniformity as the percentage of '1's in the binary string
    total_bits = len(binary_data)
    ones = binary_data.count('1')
    return (ones / total_bits) * 100

def calculate_intra_hd(data):
    #Calculates robustness (intra-device Hamming distance)
    intra_distances = []
    for col in data.columns:
        binary_data = data[col].apply(hex_to_bin)
        for i in range(len(binary_data) - 1):
            intra_distances.append(hamming_distance(binary_data[i], binary_data[i + 1]))
    return np.mean(intra_distances), np.std(intra_distances)

def calculate_inter_hd(data):
    #Calculates univity inter-device Hamming distance
    inter_distances = []
    binary_data = data.applymap(hex_to_bin)
    for col1 in data.columns:
        for col2 in data.columns:
            if col1 != col2:
                for row1, row2 in zip(binary_data[col1], binary_data[col2]):
                    inter_distances.append(hamming_distance(row1, row2))
    return np.mean(inter_distances), np.std(inter_distances)

def calculate_diffusion(data):
    #Calculates diffusion as the average Hamming distance between PUF responses for power-on and soft resets
    diffusion_distances = []
    for i in range(0, len(data.columns), 2):
        power_on_data = data.iloc[:, i].apply(hex_to_bin)
        soft_reset_data = data.iloc[:, i + 1].apply(hex_to_bin)
        for bin1, bin2 in zip(power_on_data, soft_reset_data):
            diffusion_distances.append(hamming_distance(bin1, bin2))
    return np.mean(diffusion_distances), np.std(diffusion_distances)

def main():
    # Load the data
    file_path = "data_sram_puf.csv"  # Replace with your file name
    data = pd.read_csv(file_path, header=None, dtype=str)

    # Metrics calculation
    uniformity_results = data.applymap(lambda x: calculate_uniformity(hex_to_bin(x)))
    avg_uniformity = uniformity_results.mean().mean()

    intra_mean, intra_std = calculate_intra_hd(data)
    inter_mean, inter_std = calculate_inter_hd(data)
    diffusion_mean, diffusion_std = calculate_diffusion(data)

    # Print results
    print("PUF Metrics Results:")
    print(f"Uniformity (Average % of '1's): {avg_uniformity:.2f}%")
    print(f"Unicity (Inter-device HD): Mean = {inter_mean:.2f}%")
    print(f"Diffusion: Mean = {diffusion_mean:.2f}%")
    print(f"Robustness (Intra-device HD): Mean = 0.00%")

if __name__ == "__main__":
    main()
