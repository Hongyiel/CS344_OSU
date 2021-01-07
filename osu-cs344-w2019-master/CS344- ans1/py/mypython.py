# Assignment py
# Christopher Rico
# CS344
# OSU ID: 933239746

import random
import string

#random lowercase letter generator
def key_gen():
    keylist = random.choice(string.ascii_lowercase)
    return keylist

#open all three files for output
file_1 = open("file1", 'w')
file_2 = open("file2", 'w')
file_3 = open("file3", 'w')

#put them into an array
files = [file_1, file_2, file_3]

#for each file in the array, generate 10 random lowercase letters
for file in files:

    #generate the 10 letters
    randString = ''
    for x in range (0, 10):
        randString = randString + key_gen()

    #print the letters to the screen
    print(randString)

    #save the letters to the file, add a newline, then close the file
    file.write(randString)
    file.write('\n')
    file.close()


#generate random numbers and get their product
from random import randint
num1 = randint(1,42)
num2 = randint(1,42)
product = num1 * num2

#print them to the screen
print(num1)
print(num2)
print(product)
