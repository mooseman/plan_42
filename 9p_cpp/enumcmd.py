

#  enumcmd.py 
#  The first part of Tom Newsham's 9p implementation. 

cmdName = {}
def enumCmd(*args) :
	num = 100
	ns = globals()
	for name in args :
		cmdName[num] = "T%s" % name
		cmdName[num+1] = "R%s" % name
		ns["T%s" % name] = num
		ns["R%s" % name] = num+1
		num += 2
        # print ns
	ns["Tmax"] = num

enumCmd("version", "auth", "attach", "error", "flush", "walk", "open",
		"create", "read", "write", "clunk", "remove", "stat", "wstat")
        
for x in cmdName: 
  print x 
  
'''foo = globals() 
print foo ''' 


 

  
      
  
        
