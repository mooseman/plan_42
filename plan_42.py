 
#  Have you been looking for a public domain operating system? 
#  The perfect public domain OS? One which treats everything as a file? 

#  Well, your search will have to go on for a bit longer. In the 
#  meantime though, you can play with Plan 42.....  ;)    

#  This code is released to the publ;ic domain.  

#  This code requires Teagit.  


#  Teagit will be the toy "file system".  

#  Note - to open a file in vi and go to a given 
#  line, do this - vi +5 myfile.txt - goes to line 5. 

# from teagit import * 


#  A logon class 
class logon(): 
  def __init__(self, os, username, pw): 
      self.os = os 
      self.username = username 
      self.pw = pw 
	 
      os = os.lower()
	 
      if "win" in os: 
	      print "Oh, come on now, be serious - use a REAL OS!" 
		
		 
#  Run the class 
a = logon("win", "fred", "foobar") 







   












