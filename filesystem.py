

#  filesystem.py 

#  A class to create a pseudo-filesystem.  

#  This code is released to the public domain 

class fs(object): 
   def init(self):   
      self.fsdict = {} 
      self.inode = 0
      
   # Create a filesystem       
   def mkfs(self, size): 
      self.inode += size 
              
   # Add a file to the filesystem 
   def add(self, fname): 
      self.fname = fname 
      self.fsdict[self.inode] = self.fname 
            
   def display(self): 
      print self.fsdict 
      

#  Test the class 
a = fs() 
a.init() 
a.mkfs(300) 
a.add("foo.txt")
a.add("bar.png")
a.display() 

                            
                            
                            
