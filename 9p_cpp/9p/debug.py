
#!/usr/bin/python

def exceptions(type, value, tb) :
	import pdb
	import traceback
	traceback.print_exception(type, value, tb)
	print
	pdb.pm()

import sys
sys.excepthook = exceptions

