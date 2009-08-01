

#  A class to manage users 

#  This code is released to the public domain 

class prefs(object): 
   def init(self): 
      self.prefdict = {} 
      self.userid = 0 
            
   def add(self, username): 
      self.username = username      
      self.userid += 1  
      self.prefdict[self.userid] = self.username
      
   # Modify a users preferences. Note that self.prefdict[userid] here
   # is simply the value already assigned to that key: in other words, 
   # it is the name of the user.           
   def modify(self, userid, apptype, app): 
      if self.prefdict.has_key(userid): 
         self.prefdict[userid] = [self.prefdict[userid], apptype, app]    
      else: 
         print "Error: User is not in system."  
                              
   def display(self): 
      print self.prefdict 
      
#  Test the class 
a = prefs() 
a.init() 
a.add("Joe Bloggs") 
a.add("Mary Bloggs") 
a.add("Fred Flintstone") 

a.modify(1, "editor", "vi") 
a.modify(2, "editor", "vi") 
a.modify(3, "editor", "nano")  
a.modify(2, "browser", "Firefox") 
a.display()                   



	   
