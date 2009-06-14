#!/usr/bin/python 

import sys
import socket
import os.path
import copy

import P9
import P9sk1

nochg2 = 0xffff
nochg4 = 0xffffffffL
nochg8 = 0xffffffffffffffffL
nochgS = ''

ServError = P9.ServError
class Error(P9.Error) : pass
#class ServError(Exception) : pass

def normpath(p) :
	return os.path.normpath(os.path.abspath(p))

def hash8(obj) :
	return abs(hash(obj))

def uidname(u) :			# XXX
	return "%d" % u
gidname = uidname			# XXX

def _os(func, *args) :
	try :
		return func(*args)
	except OSError,e :
		raise ServError(e.args[1])
	except IOError,e :
		raise ServError(e.args[1])

def _nf(func, *args) :
	try :
		return func(*args)
	except ServError,e :
		return

dbg = 0

# maps path names to filesystem objects
mountTable = {}
klasses = {}

def mount(path, obj) :
	"""
	Mount obj at path in the tree.  Path should exist and be a directory.
	Only one instance of obj of a given type is allowed since otherwise
	they would compete for the same storage in the File object.
	"""
	k = obj.__class__
	if k in klasses :
		raise Error("only one %s allowed" % k)
	# XXX walk tree to ensure that mountpoint exists and is
	# a directory!
	path = normpath(path)
	if path not in mountTable :
		mountTable[path] = []
	mountTable[path].append(obj)
	klasses[k] = 1


class File(object) :
	"""
	A File object represents an instance of a file, directory or path.
	It contains all the per-instance state for the file/dir/path.
	It is associated with a filesystem object (or occasionally with
	multiple filesystem objects at union mount points).  All file instances
	implemented by a filesystem share a single file system object.
	"""
	def __init__(self, path, dev=None, parent=None) :
		"""If dev is specified this must not be the root of the dev."""
		self.path = normpath(path)
		self.basename = os.path.basename(self.path)
		self.parent = parent
		self.isdir = 0
		self.dirlist = []
		self.odev = None

		self.devs = []
		if dev :
			self.devs.append(dev)
			dev.estab(self, 0)
		if self.path in mountTable :
			for d in mountTable[self.path] :
				self.devs.append(d)
				d.estab(self, 1)
		if not self.devs :
			raise ServError("no implementation for %s" % self.path)
		self.dev = self.devs[0]

	def _checkOpen(self, want) :
		if (self.odev is not None) != want :
			err = ("already open", "not open")[want]
			raise ServError(err)

	def dup(self) :
		"""
		Dup a non-open object.  
		N.B. No fields referenced prior to opening the file can be altered!
		"""
		self._checkOpen(0)
		return copy.copy(self)

	def getQid(self) :
		type = self.dev.type
		if self.isdir :
			type |= P9.QDIR
		return type,0,hash8(self.path)

	def walk(self, n) :
		self._checkOpen(0)
		path = os.path.join(self.path, n)
		for d in self.devs :
			fn = File(path, d, self)
			if d.walk(self, fn, n) :
				return fn

	def _statd(self, d) :
		s = list(d.stat(self))
		q = self.getQid()
		s[1] = q[0]
		s[3] = q
		s[8] = self.basename
		return s

	def stat(self) :
		# XXX return all stats or just the first one?
		return self._statd(self.dev)

	def wstat(self, stbuf) :
		self._checkOpen(0)
		self.dev.wstat(self, stbuf)
		l,t,d,q,mode,at,mt,sz,name,uid,gid,muid = st
		if name is not nochgS :
			new = normpath(os.path.join(os.path.basedir(self.path), name))
			self.path = new

	def remove(self) :
		# XXX checkOpen?
		if self.path in mountTable :
			raise ServError("mountpoint busy")
		self.dev.remove(self)

	def open(self, mode) :
		self._checkOpen(0)
		for d in self.devs :
			d.open(self, mode)
			self.odev = d

	def create(self, n, perm, mode) :
		self._checkOpen(0)
		path = os.path.join(self.path, n)
		for d in self.devs :
			fn = File(path, d, self)
			if d.exists(fn) :
				raise ServError("already exists")
		for d in self.devs :
			fn = File(path, d, self)
			if d.cancreate :
				d.create(fn, perm, mode)
				fn.odev = d
				return fn
		raise ServError("creation not allowed")

	def clunk(self) :
		if self.odev :
			self.odev.clunk(self)
			self.odev = None

	def _readDir(self, off, l) :
		if off == 0 :
			self.dirlist = []
			for d in self.devs :
				for n in d.list(self) :
					# XXX ignore exceptions in stat?
					path = os.path.join(self.path, n)
					fn = File(path, d, self)
					s = fn._statd(d)
					self.dirlist.append(s)
		# otherwise assume we continue where we left off
		p9 = P9.Marshal9P(None)
		p9.setBuf()
		while self.dirlist :
			# Peeking into our abstractions here.  Proceed cautiously.
			xl = len(p9.bytes)
			p9._encStat(self.dirlist[0:1], enclen=0)
			if len(p9.bytes) > l :			# backup if necessary
				p9.bytes = p9.bytes[:xl]
				break
			self.dirlist[0:1] = []
		return p9.getBuf()

	def read(self, off, l) :
		self._checkOpen(1)
		if self.isdir :
			return self._readDir(off, l)
		else :
			return self.odev.read(self, off, l)

	def write(self, off, buf) :
		self._checkOpen(1)
		if self.isdir :
			raise ServError("can't write directories")
		return self.odev.write(self, off, buf)

class AuthFs(object) :
	"""
	A special file for performing p9sk1 authentication.  On completion
	of the protocol, suid is set to the authenticated username.
	"""
	type = ord('a')
	HaveProtos,HaveSinfo,HaveSauth,NeedProto,NeedCchal,NeedTicket,Success = range(7)
	cancreate = 0

	def __init__(self, user, dom, key) :
		self.sk1 = P9sk1.Marshal()
		self.user = user
		self.dom = dom
		self.ks = key

	def estab(self, f, isroot) :
		f.isdir = 0
		f.odev = self
		f.CHs = P9sk1.randChars(8)
		f.CHc = None
		f.suid = None
		f.treq = [P9sk1.AuthTreq, self.user, self.dom, f.CHs, '', '']
		f.phase = self.HaveProtos

	def _invalid(self, *args) :
		raise ServError("bad operation")
	walk = _invalid
	remove = _invalid
	create = _invalid
	open = _invalid

	def exists(self, f) :
		return 1
	def clunk(self, f) :
		pass

	def read(self, f, pos, len) :
		self.sk1.setBuf()
		if f.phase == self.HaveProtos :
			f.phase = self.NeedProto
			return "p9sk1@%s\0" % self.dom
		elif f.phase == self.HaveSinfo :
			f.phase = self.NeedTicket
			self.sk1._encTicketReq(f.treq)
			return self.sk1.getBuf()
		elif f.phase == self.HaveSauth :
			f.phase = self.Success
			self.sk1._encAuth([P9sk1.AuthAs, f.CHc, 0])
			return self.sk1.getBuf()
		raise ServError("unexpected phase")

	def write(self, f, pos, buf) :
		self.sk1.setBuf(buf)
		if f.phase == self.NeedProto :
			l = buf.index("\0")
			if l < 0 :
				raise ServError("missing terminator")
			s = buf.split(" ")
			if len(s) != 2 or s[0] != "p9sk1" or s[1] != self.dom + '\0' :
				raise ServError("bad protocol %r" % buf)
			f.phase = self.NeedCchal
			return l + 1
		elif f.phase == self.NeedCchal :
			f.CHc = self.sk1._decChal()
			f.phase = self.HaveSinfo
			return 8
		elif f.phase == self.NeedTicket :
			self.sk1.setKs(self.ks)
			num,chal,cuid,suid,key = self.sk1._decTicket()
			if num != P9sk1.AuthTs or chal != f.CHs :
				raise ServError("bad ticket")
			self.sk1.setKn(key)
			num,chal,id = self.sk1._decAuth()
			if num != P9sk1.AuthAc or chal != f.CHs or id != 0 :
				raise ServError("bad authentication for %s" % suid)
			f.suid = suid
			f.phase = self.HaveSauth
			return 72 + 13
		raise ServError("unexpected phase")

class LocalFs(object) :
	"""
	A local filesystem device.
	"""
	type = ord('f')
	def __init__(self, root, cancreate=1) :
		self.root = normpath(root)
		self.cancreate = cancreate

	def estab(self, f, isroot) :
		if isroot :
			f.localpath = self.root
		else :
			f.localpath = normpath(os.path.join(f.parent.localpath, f.basename))
		f.isdir = os.path.isdir(f.localpath)
		f.fd = None

	def walk(self, f, fn, n) :
		if os.path.exists(fn.localpath) :
			return fn

	def remove(self, f) :
		if f.isdir :
			_os(os.rmdir, f.localpath)
		else :
			_os(os.remove, f.localpath)

	def stat(self, f) :
		s = _os(os.stat, f.localpath)
		u = uidname(s.st_uid)
		return (0, 0, s.st_dev, None, s.st_mode & 0777, 
				int(s.st_atime), int(s.st_mtime),
				s.st_size, None, u, gidname(s.st_gid), u)

	def wstat(self, f, st) :
		# nowhere near atomic
		l,t,d,q,mode,at,mt,sz,name,uid,gid,muid = st
		s = _os(os.stat, f.localpath)
		if sz != nochg8 :
			raise ServError("size changes unsupported")		# XXX
		if (uid,gid,muid) != (nochgS,nochgS,nochgS) :
			raise ServError("user change unsupported")		# XXX
		if name != nochgS :
			new = os.path.join(os.path.basedir(f.localpath), name)
			_os(os.rename, f.localpath, new)
			f.localpath = new
		if mode != nochg4 :
			_os(os.chmod, f.localpath, mode & 0777)

	def create(self, f, perm, mode) :
		# nowhere close to atomic. *sigh*
		if perm & P9.DIR :
			_os(os.mkdir, f.localpath, perm & ~P9.DIR)
			f.isdir = 1
		else :
			_os(file, f.localpath, "w+").close()
			_os(os.chmod, f.localpath, perm & 0777)
			f.isdir = 0
		return self.open(f, mode)
		
	def exists(self, f) :
		return os.path.exists(f.localpath)

	def open(self, f, mode) :
		if not f.isdir :
			if (mode & 3) == P9.OWRITE :
				if mode & P9.OTRUNC :
					m = "wb"
				else :
					m = "r+b"		# almost
			elif (mode & 3) == P9.ORDWR :
				if m & OTRUNC :
					m = "w+b"
				else :
					m = "r+b"
			else :				# P9.OREAD and otherwise
				m = "rb"
			f.fd = _os(file, f.localpath, m)

	def clunk(self, f) :
		if f.fd is not None :
			f.fd.close()
			f.fd = None

	def list(self, f) :
		l = os.listdir(f.localpath)
		return filter(lambda x : x not in ('.','..'), l)

	def read(self, f, pos, l) :
		f.fd.seek(pos)
		return f.fd.read(l)

	def write(self, f, pos, buf) :
		f.fd.seek(pos)
		f.fd.write(buf)
		return len(buf)

class Server(P9.RpcServer) :
	"""
	A tiny 9p server.
	"""
	BUFSZ = 8320

	def __init__(self, fd, user, dom, key) :
		P9.RpcServer.__init__(self, fd)

		self.authfs = AuthFs(user, dom, key)
		self.root = File('/')
		self.fid = {}

	def _getFid(self, fid) :
		if fid not in self.fid :
			raise ServError("fid %d not in use" % fid)
		obj = self.fid[fid]
		return obj

	def _setFid(self, fid, obj) :
		if fid in self.fid :
			raise ServError("fid %d in use" % fid)
		self.fid[fid] = obj
		return obj

	def _walk(self, obj, path) :
		qs = []
		for p in path :
			if p == '/' :
				obj = self.root
			elif p == '..' :
				obj = obj.parent
			else :
				if p.find('/') >= 0 :
					raise ServError("illegal character in file")
				obj = obj.walk(p)
			if obj is None :
				break
			qs.append(obj.getQid())
		return qs,obj

	def _srvTversion(self, type, tag, vals) :
		bufsz,vers = vals
		if vers != P9.version :
			raise ServError("unknown version %r" % vers)
		if bufsz > self.BUFSZ :
			bufsz = self.BUFSZ
		return bufsz,vers

	def _srvTauth(self, type, tag, vals) :
		fid,uname,aname = vals
		obj = File('#a', self.authfs)
		self._setFid(fid, obj)
		return (obj.getQid(),)

	def _srvTattach(self, type, tag, vals) :
		fid,afid,uname,aname = vals
		a = self._getFid(afid)
		if a.suid != uname :
			raise ServError("not authenticated as %r" % uname)
		r = self._setFid(fid, self.root.dup())
		return (r.getQid(),)

	def _srvTflush(self, type, tag, vals) :
		return ()

	def _srvTwalk(self, type, tag, vals) :
		fid,nfid,names = vals
		obj = self._getFid(fid)
		qs,obj = self._walk(obj, names)
		if len(qs) == len(names) :
			self._setFid(nfid, obj)
		return qs,

	def _srvTopen(self, type, tag, vals) :
		fid,mode = vals
		obj = self._getFid(fid).dup()
		obj.open(mode)
		self.fid[fid] = obj
		return obj.getQid(),4096		# XXX

	def _srvTcreate(self, type, tag, vals) :
		fid,name,perm,mode = vals
		obj = self._getFid(fid)
		obj = obj.create(name, perm, mode)
		self.fid[fid] = obj
		return obj.getQid(),4096		# XXX

	def _srvTread(self, type, tag, vals) :
		fid,off,count = vals
		return self._getFid(fid).read(off, count),

	def _srvTwrite(self, type, tag, vals) :
		fid,off,data = vals
		return self._getFid(fid).write(off, data),

	def _srvTclunk(self, type, tag, vals) :
		fid = vals
		self._getFid(fid).clunk()
		del self.fid[fid]
		return None,

	def _srvTremove(self, type, tag, vals) :
		fid = vals
		obj = self._getFid(fid)
		# clunk even if remove fails
		r = self._srvTclunk(type, tag, vals)
		obj.remove()
		return r

	def _srvTstat(self, type, tag, vals) :
		# XXX to return multiple stat entries or not?!
		fid = vals
		obj = self._getFid(fid)
		return [obj.stat()],

	def _srvTwstat(self, type, tag, vals) :
		fid,stats = vals
		if len(stats) != 1 :
			raise ServError("multiple stats")
		obj = self._getFid(fid)
		obj.wstat(stats[0])
		return None,

def sockserver(user, dom, key, port=P9.PORT) :
	sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
	sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
	sock.bind(('0.0.0.0', port),)
	sock.listen(1)
	while 1 :
		sock2,addr = sock.accept()
		# XXX fork is breaking in cygwin, looks like a bug, possibly
		# due to importing the crypto dll?
		if dbg or not hasattr(os, 'fork') or sys.platform == 'cygwin' or os.fork() == 0 :
			sock.close()
			break
		sock2.close()

	try :
		print "serving: %r,%r" % addr
		s = Server(P9.Sock(sock2), user, dom, key) 
		s.serve()
		print "done serving %r,%r" % addr
	except P9.Error,e :
		print e.args[0]

def usage(prog) :
	print "usage:  %s [-d] [-m module] [-p port] [-r root] srvuser domain" % prog
	sys.exit(1)

def main(prog, *args) :
	import getopt
	import getpass

	port = P9.PORT
	root = '/'
	mods = []
	try :
		opt,args = getopt.getopt(args, "dm:p:r:")
	except :
		usage(prog)
	for opt,optarg in opt :
		if opt == "-d" :
			global dbg
			import debug
			dbg = 1
		if opt == '-m' :
			mods.append(optarg)
		if opt == '-r' :
			root = optarg
		if opt == "-p" :
			port = int(optarg)

	if len(args) < 2 :
		usage(prog)
	user = args[0]
	dom = args[1]
	passwd = getpass.getpass()
	key = P9sk1.makeKey(passwd)

	mount('/', LocalFs(root))
	for m in mods :
		x = __import__(m)
		mount(x.mountpoint, x.root)
		print '%s loaded.' % m
	sockserver(user, dom, key, port)

if __name__ == "__main__" :
	try :
		main(*sys.argv)
	except KeyboardInterrupt :
		print "interrupted."

