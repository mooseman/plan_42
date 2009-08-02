

#  plumber.py 
#  A simple "toy plumber" class.  
#  This code is released to the public domain. 

#  This version of the plumber code uses an 
#  ordinary dictionary (not 3.1's OrderedDict), 
#  so it can be run with most versions of Python. 

#  Note - to open a file in vi and go to a given 
#  line, do this - vi +5 myfile.txt - goes to line 5. 

#  Note - this command works! It opens nano as expected, on the 
#  5th line of the file - 
#  pipe = subprocess.Popen('nano +5 foo.txt', shell=True, bufsize=2000).stdout 
#  The same command also works with vi.  

#  TO DO - Add code to look at the various plumbing rules 
#  and act on them.  


import os, fileinput, itertools, subprocess     

class readplumbrules(object): 
   def init(self):  
      self.mydict = {} 
	  
   def open(self, rulesfile):
      self.file = open(rulesfile, 'r').readlines()
      for k, v in enumerate(self.file): 
	      self.mydict[k] = v.rstrip('\n') 
		  
   def display(self): 
      print self.mydict 
	  	  

#  Run the class 
'''a = readplumbrules() 
a.init() 
a.open('plumbertest1') 
a.display() '''


#  Now, a class to take the read-in rules and act on them.  
class runrules(readplumbrules): 
   # define the apps to run the commands 
   def apps(self): 
      self.editor = 'nano' 
                  
   def run(self, file): 
      self.fname = file 
      if self.fname.endswith(self, '.txt', start=0): 
         pipe = subprocess.Popen("self.editor" '+5' "self.fname", shell=True, bufsize=2000).stdout 
      else: 
         pass  
         
#  Test the class 
a = runrules() 
print dir(a) 

a.init() 

a.open('plumbertest1') 
a.display() 


                  




