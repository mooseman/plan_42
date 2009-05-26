

#  plumber.py 
#  A simple "toy plumber" class.  
#  This code is released to the public domain. 

#  Note - to open a file in vi and go to a given 
#  line, do this - vi +5 myfile.txt - goes to line 5. 

#  Need the following functions - 
#  read_plumb_file, 

import os, fileinput  

class plumber(): 
   @staticmethod 
   def open(): 
      os.system('vi +5 foo.txt') 
	  

#  Run the class 
# myplumb = plumber()

# myplumb.open() 


f = open('plumbertest1', 'rb')

for line in f: 
   print line.splitlines() 
   

	  
   
   
   

