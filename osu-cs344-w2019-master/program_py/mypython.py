# mypython.py
#
# Phi Luu
#
# Oregon State University
# CS_344_001_W2019 Operating Systems 1
# Program Py: Python Exploration

import os
import random
import string

def generate_string(length):
    """Generates a random string consisted of lowercase ASCII characters.
    Credits: https://stackoverflow.com/a/2257449.

    Args:
        length: The desired length of this random string

    Returns:
        A randomly generated lowercase ASCII string of length  length .
    """
    return ''.join(random.choice(string.ascii_lowercase) for _ in range(length))

# create/open three temporary files in write mode
for i in range(3):
    with open('f' + str(i + 1), 'w') as file:
        # generate a random lowercase ASCII string of length 10, followed by EOL
        file_content = generate_string(10) + '\n'

        # write the content to the file
        file.write(file_content)

        # output to stdout the content of the file
        print(file_content, end='')

# print to stdout two random integers between [1, 42] and their product
rand_int1 = random.randint(1, 42)
print(str(rand_int1))
rand_int2 = random.randint(1, 42)
print(str(rand_int2))
print(str(rand_int1 * rand_int2))
