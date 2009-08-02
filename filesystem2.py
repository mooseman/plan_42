

#  filesystem.py 

#  A class to create a pseudo-filesystem.  

#  This code is released to the public domain 

class fs(object): 
   def init(self):   
      self.fsdict = {} 
      self.size = 0
              
   # Add a file to the filesystem 
   def add(self, fname, size): 
      self.fname = fname 
      self.size = size 
      self.fsdict[self.size] = self.fname 
            
   def display(self): 
      print self.fsdict 
      

#  Test the class 
a = fs() 
a.init() 
a.add("foo.txt", 350)
a.add("bar.png", 540)
a.display() 

                            
                            
                            
