

#  plumber.py 
#  A simple "toy plumber" class.  
#  This code is released to the public domain. 

#  NOTE! - This code uses an OrderedDict, and so 
#  you must use Python 3.1 or later to run it. 

#  Note - to open a file in vi and go to a given 
#  line, do this - vi +5 myfile.txt - goes to line 5. 

#  TO DO - Add code to look at the various plumbing rules 
#  and act on them.  


import os, fileinput, collections, itertools    

class plumber(): 
   def init(self):  
      self.mydict = collections.OrderedDict() 
	  
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

   

