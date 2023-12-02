import os
import random

# Function to generate random slices from the given string
def generate_slices(input_string, num_slices, slice_length, file_path):
    slices = []
    max_start_index = len(input_string) - slice_length
    for _ in range(num_slices):
        start_index = random.randint(0, max_start_index)
        slices.append(input_string[start_index:start_index+slice_length])
    # Write to file
    with open(file_path, 'w') as file:
        file.writelines("%s\n" % slice for slice in slices)

# Read the original file content
with open('string1000000.txt', 'r') as file:
    original_string = file.read().strip()

# Generate the string files with random slices
string_lengths = [500000, 100000, 50000, 10000, 5000, 1000]
pattern_lengths = [500, 400, 300, 200, 100]

# Generating smaller string files
for length in string_lengths:
    # Random start index for slicing
    start_index = random.randint(0, len(original_string) - length)
    # Generate the slice
    slice = original_string[start_index:start_index + length]
    # Define the file name
    file_name = f'string{length}.txt'
    # Write the slice to a new file
    with open(file_name, 'w') as file:
        file.write(slice)

# Generating pattern files
for length in pattern_lengths:
    # Define the file name
    file_name = f'patterns{length}.txt'
    # Generate and write patterns to a new file
    generate_slices(original_string, 100, length, file_name)
