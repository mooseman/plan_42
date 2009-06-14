

#  plumber.py 
#  A simple "toy plumber" class.  
#  This code is released to the public domain. 

#  This version of the plumber code uses an 
#  ordinary dictionary (not 3.1's OrderedDict), 
#  so it can be run with most versions of Python. 

#  Note - to open a file in vi and go to a given 
#  line, do this - vi +5 myfile.txt - goes to line 5. 

#  TO DO - Add code to look at the various plumbing rules 
#  and act on them.  


import os, fileinput, itertools    

class plumber(): 
   def init(self):  
      self.mydict = {} 
	  
   def open(self, file):
      self.file = open(file, 'r').readlines()
      for k, v in enumerate(self.file): 
	      self.mydict[k] = v.rstrip('\n') 
		  
   def display(self): 
      print(self.mydict) 
	  	  

#  Run the class 
myplumb = plumber() 

myplumb.init() 

myplumb.open('plumbertest1') 

myplumb.display() 

   

