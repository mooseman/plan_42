 
#  Have you been looking for a public domain operating system? 
#  The perfect public domain OS? One which treats everything as a file? 

#  Well, your search will have to go on for a bit longer. In the 
#  meantime though, you can play with Plan 42.....  ;)    

#  This code is released to the public domain.  


import time 

#  A logon class.  This will be improved using 
#  getpass.py and hashlib soon.  
class logon(): 
  def __init__(self, username, pw):       
      self.username = username 
      self.pw = pw 
	 
	  # Do some stuff to imitate setup during boot 
      print "Welcome to Plan 42! Share and enjoy.... " 
      time.sleep(3) 
      print "Mounting drives... please wait..." 
      time.sleep(3) 
      print "Mounted /  /dev  /usr  /etc  "  
      time.sleep(3) 
      print "Done!"  
	 
      		
#  Run the class 
a = logon("fred", "foobar") 










   












