#!/usr/bin/python

import socket
import sys

import P9

class Error(P9.Error) : pass

def modeStr(mode) :
	bits = ["---", "--x", "-w-", "-wx", "r--", "r-x", "rw-", "rwx"]
	def b(s) :
		return bits[(mode>>s) & 7]
	d = "-"
	if mode & P9.DIR :
		d = "d"
	return "%s%s%s%s" % (d, b(6), b(3), b(0))

def _os(func, *args) :
	try :
		return func(*args)
	except OSError,e :
		raise Error(e.args[1])
	except IOError,e :
		raise Error(e.args[1])
	
class Client(object) :
	"""
	A tiny 9p client.
	"""
	AFID = 10
	ROOT = 11
	CWD = 12
	F = 13

	def __init__(self, fd, user, passwd, authsrv) :
		self.rpc = P9.RpcClient(fd)
		self.login(user, passwd, authsrv)

	def login(self, user, passwd, authsrv) :
		maxbuf,vers = self.rpc.version(16 * 1024, P9.version)
		if vers != P9.version :
			raise Error("version mismatch: %r" % vers)

		afid = self.AFID
		try :
			self.rpc.auth(afid, user, '')
			needauth = 1
		except P9.RpcError,e :
			afid = P9.nofid

		if afid != P9.nofid :
			if passwd is None :
				raise Error("Password required")

			import P9sk1
			try :
				P9sk1.clientAuth(self.rpc, afid, user, P9sk1.makeKey(passwd), authsrv, P9sk1.AUTHPORT)
			except socket.error,e :
				raise Error("%s: %s" % (authsrv, e.args[1]))
		self.rpc.attach(self.ROOT, afid, user, "")
		if afid != P9.nofid :
			self.rpc.clunk(afid)
		self.rpc.walk(self.ROOT, self.CWD, [])

	def close(self) :
		self.rpc.clunk(self.ROOT)
		self.rpc.clunk(self.CWD)
		self.sock.close()

	def _walk(self, pstr='') :
		root = self.CWD
		if pstr == '' :
			path = []
		else :
			path = pstr.split("/")
			if path[0] == '' :
				root = self.ROOT
				path = path[1:]
			path = filter(None, path)
		try : 
			w = self.rpc.walk(root, self.F, path)
		except P9.RpcError,e :
			print "%s: %s" % (pstr, e.args[0])
			return
		if len(w) < len(path) :
			print "%s: not found" % pstr
			return
		return w
	def _open(self, pstr='', mode=0) :
		if self._walk(pstr) is None :
			return
		self.pos = 0L
		return self.rpc.open(self.F, mode)
	def _create(self, pstr, perm=0644, mode=1) :
		p = pstr.split("/")
		pstr2,name = "/".join(p[:-1]),p[-1]
		if self._walk(pstr2) is None :
			return
		self.pos = 0L
		try :
			return self.rpc.create(self.F, name, perm, mode)
		except P9.RpcError,e :
			self._close()
			raise P9.RpcError(e.args[0])
	def _read(self, l) :
		buf = self.rpc.read(self.F, self.pos, l)
		self.pos += len(buf)
		return buf
	def _write(self, buf) :
		l = self.rpc.write(self.F, self.pos, buf)
		self.pos += l
		return l
	def _close(self) :
		self.rpc.clunk(self.F)

	def stat(self, pstr) :
		if self._walk(pstr) is None :
			print "%s: not found" % pstr
		else :
			for sz,t,d,q,m,at,mt,l,name,u,g,mod in self.rpc.stat(self.F) :
				print "%s %s %s %-8d\t%s" % (modeStr(m), u, g, l, name)
			self._close()
		
	def ls(self, long=0) :
		if self._open() is None :
			return
		while 1 :
			buf = self._read(4096)
			if len(buf) == 0 :
				break
			p9 = self.rpc.msg
			p9.setBuf(buf)
			for sz,t,d,q,m,at,mt,l,name,u,g,mod in p9._decStat(0) :
				if long :
					print "%s %s %s %-8d\t%s" % (modeStr(m), u, g, l, name)
				else :
					print name,
		if not long :
			print
		self._close()
	def cd(self, pstr) :
		q = self._walk(pstr)
		if q is None :
			return
		if q and not (q[-1][0] & P9.QDIR) :
			print "%s: not a directory" % pstr
			self._close()
			return
		self.F,self.CWD = self.CWD,self.F
		self._close()

	def mkdir(self, pstr, perm=0644) :
		self._create(pstr, perm | P9.DIR)
		self._close()

	def cat(self, name, out=None) :
		if out is None :
			out = sys.stdout
		if self._open(name) is None :
			return
		while 1 :
			buf = self._read(4096)
			if len(buf) == 0 :
				break
			out.write(buf)
		self._close()
	def put(self, name, inf=None) :
		if inf is None :
			inf = sys.stdin
		x = self._create(name)
		if x is None :
			x = self._open(name, P9.OWRITE|P9.OTRUNC)
			if x is None :
				return
		sz = 1024
		while 1 :
			buf = inf.read(sz)
			self._write(buf)
			if len(buf) < sz :
				break
		self._close()
	def rm(self, pstr) :
		self._open(pstr)
		self.rpc.remove(self.F)

class CmdClient(Client) :
	"""command line driven access to the client"""
	def _cmdstat(self, args) :
		for a in args :
			self.stat(a)
	def _cmdls(self, args) :
		long = 0
		while len(args) > 0 :
			if args[0] == "-l" :
				long = 1
			else :
				print "usage: ls [-l]"
				return
			args[0:1] = []
		self.ls(long)
	def _cmdcd(self, args) :
		if len(args) != 1 :
			print "usage: cd path"
			return
		self.cd(args[0])
	def _cmdcat(self, args) :
		if len(args) != 1 :
			print "usage: cat path"
			return
		self.cat(args[0])
	def _cmdmkdir(self, args) :
		if len(args) != 1 :
			print "usage: mkdir path"
			return
		self.mkdir(args[0])
	def _cmdget(self, args) :
		if len(args) == 1 :
			f, = args
			f2 = f.split("/")[-1]
		elif len(args) == 2 :
			f,f2 = args
		else :
			print "usage: get path [localname]"
			return
		out = _os(file, f2, "wb")
		self.cat(f, out)
		out.close()
	def _cmdput(self, args) :
		if len(args) == 1 :
			f, = args
			f2 = f.split("/")[-1]
		elif len(args) == 2 :
			f,f2 = args
		else :
			print "usage: put path [remotename]"
			return
		if f == '-' :
			inf = sys.stdin
		else :
			inf = _os(file, f, "rb")
		self.put(f2, inf)
		if f != '-' :
			inf.close()
	def _cmdrm(self, args) :
		if len(args) == 1 :
			self.rm(args[0])
		else :
			print "usage: rm path"
	def _cmdhelp(self, args) :
		cmds = [x[4:] for x in dir(self) if x[:4] == "_cmd"]
		cmds.sort()
		print "Commands: ", " ".join(cmds)
	def _cmdquit(self, args) :
		self.done = 1
	_cmdexit = _cmdquit

	def _nextline(self) :		# generator is cleaner but not supported in 2.2
		if self.cmds is None :
			sys.stdout.write("9p> ")
			sys.stdout.flush()
			line = sys.stdin.readline()
			if line != "" :
				return line[:-1]
		else :
			if self.cmds :
				x,self.cmds = self.cmds[0],self.cmds[1:]
				return x
	def cmdLoop(self, cmds) :
		cmdf = {}
		for n in dir(self) :
			if n[:4] == "_cmd" :
				cmdf[n[4:]] = getattr(self, n)

		if not cmds :
			cmds = None
		self.cmds = cmds
		self.done = 0
		while 1 :
			line = self._nextline()
			if line is None :
				break
			args = filter(None, line.split(" "))
			if not args :
				continue
			cmd,args = args[0],args[1:]
			if cmd in cmdf :
				try :
					cmdf[cmd](args)
				except P9.Error,e :
					print "%s: %s" % (cmd, e.args[0])
			else :
				sys.stdout.write("%s ?\n" % cmd)
			if self.done :
				break

def usage(prog) :
	print "usage: %s [-d] [-a authsrv] [-n] [-p srvport] user srv [cmd ...]" % prog
	sys.exit(1)
	
def main(prog, *args) :
	import getopt
	import getpass

	authsrv = None
	port = P9.PORT
	try :
		opt,args = getopt.getopt(args, "a:dnp:")
	except :
		usage(prog)
	passwd = ""
	for opt,optarg in opt :
		if opt == '-a' :
			authsrv = optarg
		if opt == "-d" :
			import debug
		if opt == '-n' :
			passwd = None
		if opt == "-p" :
			port = int(optarg)		# XXX catch
	
	if len(args) < 2 :
		usage(prog)
	user = args[0]
	srv = args[1]
	if authsrv is None :
		authsrv = srv
	cmd = args[2:]

	sock = socket.socket(socket.AF_INET)
	try :
		sock.connect((srv, port),)
	except socket.error,e :
		print "%s: %s" % (srv, e.args[1])
		return

	if passwd is not None :
		passwd = getpass.getpass()
	try :
		cl = CmdClient(P9.Sock(sock), user, passwd, authsrv)
		cl.cmdLoop(cmd)
	except P9.Error,e :
		print e

if __name__ == "__main__" :
	try :
		main(*sys.argv)
	except KeyboardInterrupt :
		print "interrupted."

