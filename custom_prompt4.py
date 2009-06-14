

#  custom_prompt.py 
#  Creates a custom Python prompt to run commands from 

#  This code is released to the public domain 


import string, sys, cmd 

class myShell(cmd.Cmd): 
   prompt = 'p42>'  
   intro = 'Welcome to Plan 42!'  
   def do_exit(self, line):
      return True
   def do_quit(self, line): 
	  return True 				
   	  	
   		  		  							   		 		      
if __name__ == '__main__':
    a = myShell()  
    a.cmdloop()  
	
	





