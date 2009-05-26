 
#  Have you been looking for a public domain operating system? 
#  The perfect public domain OS? One which treats everything as a file? 

#  Well, your search will have to go on for a bit longer. In the 
#  meantime though, you can play with Plan 42.....  ;)    

#  This code is released to the publ;ic domain.  

import time 

#  A logon class 
class logon(): 
  def __init__(self, os, username, pw): 
      self.os = os 
      self.username = username 
      self.pw = pw 
	 
      os = os.lower()
	 
	  # Do some stuff to imitate setup during boot 
      print "Welcome to Plan 42! Share and enjoy.... " 
      time.sleep(3) 
      print "Mounting drives... please wait..." 
      time.sleep(3) 
      print "Mounted /  /dev  /usr  /etc  "  
      time.sleep(3) 
	 
      if "win" in os: 
          print "Oh, come on now, be serious - use a REAL OS!" 
          time.sleep(2) 
          print "Loaded BSOD()  :)  " 		

		 
#  Run the class 
a = logon("win", "fred", "foobar") 







   












