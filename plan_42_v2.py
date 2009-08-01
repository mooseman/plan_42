 
#  Have you been looking for a public domain operating system? 
#  The perfect public domain OS? One which treats everything as a file? 

#  Well, your search will have to go on for a bit longer. In the 
#  meantime though, you can play with Plan 42.....  ;)    

#  This code is released to the public domain.  

import time 
     	 
class file():  
   def init(self): 
      self.dict = {} 

   def create(self, fname, owner): 
      self.fname = fname 
      self.owner = owner 
      self.type = '' 
      self.permissions = ''  
      self.dict.update({self.fname :  [self.owner, self.type, self.permissions]} ) 
	  	  	  
   def mod(self, fname, attrib, newval): 
      if self.dict.has_key(self.fname): 
	      self.fname.attrib = newval  
          
   def display(self): 
      print self.dict           
          
	 
#  Test the class 
a = file()
a.init()
a.create("foo.txt", "Fred Bloggs")
a.create("moose.jpg", "Fred Bloggs") 
a.create("bar.txt", "Mary Bloggs") 
a.display()      
     
     
	 	 	 
class os():  
   def __init__(self, username, pw):
      self.username = username
      self.pw = pw 	  
	  # Do some stuff to imitate setup during boot 
      print "Welcome to Plan 42! Share and enjoy.... " 
      time.sleep(3) 
      print "Mounting drives... please wait..." 
      time.sleep(3) 
      print "Mounted /  /dev  /usr  /etc  "   
	  
      ''' Set up filesystem '''    
      self.filesystem = {} 
      ''' Add a few dirs to the filesystem ''' 
      self.filesystem['/'] = ["foo.txt", "bar.png", "baz.html"] 
      self.filesystem['/usr'] = ["even.txt", "more.txt", "stuff.txt"] 
      self.filesystem['/bin'] = ["cp", "mv", "rm", "mkdir", "rmdir"] 
      print self.filesystem 
      time.sleep(3) 
      print "Done!" 
	  	      	
   ''' A few commands. We will flesh these out later. ''' 			   	   
   def cp(self, source, target): 
      '''self.source = source 
	  self.target = target '''
	        
   def ls(self, dir): 
      pass 
      ''' print [fname for fname in dir]  	   	   	    ''' 
	   
   def rm(self, target): 
      ''' self.filesystem.del(target) ''' 
	   
   def mv(self, source, target): 
      ''' self.source = source 
	  self.target = target  ''' 
	  	 
	  	   	   	  		
#  Run the class 
a = os("fred", "foobar") 











   












