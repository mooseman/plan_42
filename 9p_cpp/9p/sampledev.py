#!/usr/bin/python
"""
This is a sample module.  It illustrates how to add new synthetic
filesystems to the server.
"""

import time

import P9
import srv

ServError = srv.ServError

class SampleFs(object) :
	"""
	A sample plugin filesystem.
	"""
	dirs = {
		'/' : ['sample1', 'sample2'],
	}
	type = ord('S')
	cancreate = 0
	def __init__(self) :
		self.start = int(time.time())

	def estab(self, f, isroot) :
		f.samptype = None
		if isroot :
			f.isdir = 1
			f.samptype = '/'
		else :
			pt = f.parent.samptype
			if (pt in self.dirs) and (f.basename in self.dirs[pt]) :
				f.samptype = f.basename

	def walk(self, f, fn, n) :
		if f.samptype in self.dirs and n in self.dirs[f.samptype] :
			return fn

	def remove(self, f) :
		raise ServError("bad remove")
	def stat(self, f) :
		return (0, 0, 0, None, 0644, self.start, int(time.time()),
				1024, None, 'uid', 'gid', 'muid')
	def wstat(self, f, st) :
		raise ServError("bad wstat")
	def create(self, f, perm, mode) :
		raise ServError("bad create")
	def exists(self, f) :
		return (f.samptype is not None)
	def open(self, f, mode) :
		if (mode & 0777) != P9.OREAD :
			raise ServError("permission denied")
	def clunk(self, f) :
		pass
	def list(self, f) :
		if f.samptype in self.dirs :
			return self.dirs[f.samptype]

	def read(self, f, pos, l) :
		if f.samptype == 'sample1' :
			buf = '%d\n' % time.time()
			return buf[:l]
		elif f.samptype == 'sample2' :
			buf = 'The time is now %s. thank you for asking.\n' % time.asctime(time.localtime(time.time()))
			return buf[pos : pos + l]
		return ''
		
	def write(self, f, pos, buf) :
		raise ServError('not opened for writing')

root = SampleFs()
mountpoint = '/'

