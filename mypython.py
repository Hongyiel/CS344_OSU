import random
import string

_LENGTH = 10 # length of sentence 10
_LENGTH_getfile = 4 # get file 3 times
string_pool = string.ascii_lowercase # lower_case of letter
result = "" # result1

for i in range(_LENGTH_getfile) :
	f = open("myFile", 'w')
	f.write(result)
	f.close()		
	for i in range(_LENGTH) :
		result += random.choice(string_pool) # get random string				
	if (i == 9) : 
		result += '\n'	
# get data individually

tmp_file = open("myFile", 'r')

line_first  = tmp_file.readline() # get first line info	
afile = open("myFile_first", 'w')
afile.write(line_first)	
afile.close()

line_second  = tmp_file.readline() # get second line info	
bfile = open("myFile_second", 'w')
bfile.write(line_second)
bfile.close()

line_third  = tmp_file.readline() # get thrid line info	
cfile = open("myFile_third", 'w')	
cfile.write(line_third)	
cfile.close()


tmp_file.close() # end of the file get

# print data which from random state ment
a = open("myFile_first", "r")
print(a.read(10))
b = open("myFile_second", "r")
print(b.read(10))
c = open("myFile_third", "r")
print(c.read(10))
a.close()
b.close()
c.close()
from random import *

i = randint(1,42) # 1 to 42 random number
print(i)	  # get i 
j = randint(1,42) # 1 to 42 random number
print(j)	  # get j

k = i * j	  # calculate

print(k)	  # print result
