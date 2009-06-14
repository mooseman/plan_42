#!/usr/bin/python
"""
Implementation of the p9sk1 authentication.

This module requires the Python Cryptography Toolkit from
http://www.amk.ca/python/writing/pycrypt/pycrypt.html
"""

import socket
import random
from Crypto.Cipher import DES

import P9


class Error(P9.Error) : pass
class AuthError(Error) : pass
class AuthsrvError(Error) : pass

TickReqLen = 141
TickLen = 72
AuthLen = 13

AuthTreq,AuthChal,AuthPass,AuthOK,AuthErr,AuthMod = range(1,7)
AuthTs,AuthTc,AuthAs,AuthAc,AuthTp,AuthHr = range(64, 70)

AUTHPORT = 567


_par = [ 0x01, 0x02, 0x04, 0x07, 0x08, 0x0b, 0x0d, 0x0e, 
	0x10, 0x13, 0x15, 0x16, 0x19, 0x1a, 0x1c, 0x1f, 
	0x20, 0x23, 0x25, 0x26, 0x29, 0x2a, 0x2c, 0x2f, 
	0x31, 0x32, 0x34, 0x37, 0x38, 0x3b, 0x3d, 0x3e, 
	0x40, 0x43, 0x45, 0x46, 0x49, 0x4a, 0x4c, 0x4f, 
	0x51, 0x52, 0x54, 0x57, 0x58, 0x5b, 0x5d, 0x5e, 
	0x61, 0x62, 0x64, 0x67, 0x68, 0x6b, 0x6d, 0x6e, 
	0x70, 0x73, 0x75, 0x76, 0x79, 0x7a, 0x7c, 0x7f, 
	0x80, 0x83, 0x85, 0x86, 0x89, 0x8a, 0x8c, 0x8f, 
	0x91, 0x92, 0x94, 0x97, 0x98, 0x9b, 0x9d, 0x9e, 
	0xa1, 0xa2, 0xa4, 0xa7, 0xa8, 0xab, 0xad, 0xae, 
	0xb0, 0xb3, 0xb5, 0xb6, 0xb9, 0xba, 0xbc, 0xbf, 
	0xc1, 0xc2, 0xc4, 0xc7, 0xc8, 0xcb, 0xcd, 0xce, 
	0xd0, 0xd3, 0xd5, 0xd6, 0xd9, 0xda, 0xdc, 0xdf, 
	0xe0, 0xe3, 0xe5, 0xe6, 0xe9, 0xea, 0xec, 0xef, 
	0xf1, 0xf2, 0xf4, 0xf7, 0xf8, 0xfb, 0xfd, 0xfe ]
def _expandKey(key) :
	"""Expand a 7-byte DES key into an 8-byte DES key"""
	k = map(ord, key)
	k64 = [ k[0]>>1,
			(k[1]>>2) | (k[0]<<6),
			(k[2]>>3) | (k[1]<<5),
			(k[3]>>4) | (k[2]<<4),
			(k[4]>>5) | (k[3]<<3),
			(k[5]>>6) | (k[4]<<2),
			(k[6]>>7) | (k[5]<<1),
			k[6]<<0]
	return "".join([chr(_par[x & 0x7f]) for x in k64])

def _newKey(key) :
	return DES.new(_expandKey(key), DES.MODE_ECB)

def lencrypt(key, l) :
	"""Encrypt a list of characters, returning a list of characters"""
	return list(key.encrypt("".join(l)))
def ldecrypt(key, l) :
	return list(key.decrypt("".join(l)))

def makeKey(password) :
	"""
	Hash a password into a key.
	"""
	password = password[:28-1] + '\0'
	n = len(password) - 1
	password = P9.pad(password, 28, ' ')
	buf = list(password)
	while 1 :
		t = map(ord, buf[:8])

		k = [(((t[i]) >> i) + (t[i+1] << (8-(i+1))) & 0xff) for i in xrange(7)]
		key = "".join([chr(x) for x in k])
		if n <= 8 :
			return key
		n -= 8
		if n < 8 :
			buf[:n] = []
		else :
			buf[:8] = []
		buf[:8] = lencrypt(_newKey(key), buf[:8])

def randChars(n) :
	"""
	XXX This is *NOT* a secure way to generate random strings!
	This should be fixed if this code is ever used in a serious manner.
	"""
	return "".join([chr(random.randint(0,255)) for x in xrange(n)])


class Marshal(P9.Marshal) :
	def __init__(self) :
		self.ks = None
		self.kn = None

	def setKs(self, ks) :
		self.ks = _newKey(ks)
	def setKn(self, kn) :
		self.kn = _newKey(kn)

	def _encrypt(self, n, key) :
		"""Encrypt the last n bytes of the buffer with weird chaining."""
		idx = len(self.bytes) - n
		n -= 1
		for dummy in xrange(n / 7) :
			self.bytes[idx : idx+8] = lencrypt(key, self.bytes[idx : idx+8])
			idx += 7
		if n % 7 :
			self.bytes[-8:] = lencrypt(key, self.bytes[-8:])
	def _decrypt(self, n, key) :
		"""Decrypt the first n bytes of the buffer."""
		if key is None :
			return
		m = n - 1
		if m % 7 :
			self.bytes[n-8:n] = ldecrypt(key, self.bytes[n-8:n])
		idx = m - m%7
		for dummy in xrange(m / 7) :
			idx -= 7
			self.bytes[idx : idx+8] = ldecrypt(key, self.bytes[idx : idx+8])

	def _encPad(self, x, l) :
		self._encX(P9.pad(x, l))
	def _decPad(self, l) :
		x = self._decX(l)
		idx = x.find('\0')
		if idx >= 0 :
			x = x[:idx]
		return x

	def _encChal(self, x) :
		self._checkLen(x, 8)
		self._encX(x)
	def _decChal(self) :
		return self._decX(8)

	def _encTicketReq(self, x) :
		type,authid,authdom,chal,hostid,uid = x
		self._enc1(type)
		self._encPad(authid, 28)
		self._encPad(authdom, 48)
		self._encChal(chal)
		self._encPad(hostid, 28)
		self._encPad(uid, 28)
	def _decTicketReq(self) :
		return [self._dec1(),
			self._decPad(28),
			self._decPad(48),
			self._decChal(),
			self._decPad(28),
			self._decPad(28)]

	def _encTicket(self, x) :
		num,chal,cuid,suid,key = x
		self._checkLen(key, 7)
		self._enc1(num)
		self._encChal(chal)
		self._encPad(cuid, 28)
		self._encPad(suid, 28)
		self._encX(key)
		self._encrypt(1 + 8 + 28 + 28 + 7, self.ks)

	def _decTicket(self) :
		self._decrypt(1 + 8 + 28 + 28 + 7, self.ks)
		return [self._dec1(),
			self._decChal(),
			self._decPad(28),
			self._decPad(28),
			self._decX(7)]

	def _encAuth(self, x) :
		num,chal,id = x
		self._enc1(num)
		self._encChal(chal)
		self._enc4(id)
		self._encrypt(1 + 8 + 4, self.kn)
	def _decAuth(self) :
		self._decrypt(1 + 8 + 4, self.kn)
		return [self._dec1(),
			self._decChal(),
			self._dec4()]

	def _encTattach(self, x) :
		tick,auth = x
		self._checkLen(tick, 72)
		self._encX(tick)
		self._encAuth(auth)
	def _decTattach(self) :
		return self._decX(72), self._decAuth()

def getTicket(con, sk1, treq) :
	"""
	Connect to the auth server and request a set of tickets.
	Con is an open handle to the auth server, sk1 is a handle
	to a P9sk1 marshaller with Kc set and treq is a ticket request.
	Return the (opaque) server ticket and the (decoded) client ticket.
	Raises an AuthsrvError on failure.
	"""
	sk1.setBuf()
	sk1._encTicketReq(treq)
	con.send(sk1.getBuf())
	ch = con.recv(1)
	if ch == chr(5) :
		err = con.recv(64)
		raise AuthsrvError(err)
	elif ch != chr(4) :
		raise AuthsrvError("invalid reply type %r" % ch)
	ctick = con.recv(72)
	stick = con.recv(72)
	if len(stick) + len(ctick) != 72*2 :
		raise AuthsrvError("short auth reply")
	sk1.setBuf(ctick)
	return sk1._decTicket(), stick

# this could be cleaner
def clientAuth(cl, afid, user, Kc, authsrv, authport=567) :
	"""
	Authenticate ourselves to the server.
	Cl is a P9 RpcClient, afid is the fid to use, user is the
	user name, Kc is the user's key, authsrv and authport specify
	the auth server to use for requesting tickets.

	XXX perhaps better if the auth server can be prompted for
	based on the domain in the negotiation.
	"""
	CHc = randChars(8)
	sk1 = Marshal()
	sk1.setKs(Kc)
	pos = [0]
	gen = 0

	def rd(l) :
		x = cl.read(afid, pos[0], l)
		pos[0] += len(x)
		return x
	def wr(x) :
		l = cl.write(afid, pos[0], x)
		pos[0] += l
		return l

	# negotiate
	proto = rd(128)
	v2 = 0
	if proto[:10] == 'v.2 p9sk1@' :
		v2 = 1
		proto = proto[4:]
	if proto[:6] != 'p9sk1@' :
		raise AuthError("unknown protocol %r" % proto)
	wr(proto.replace("@", " ", 1))
	if v2 :
		if rd(3) != 'OK\0' :
			raise AuthError("v.2 protocol botch")

	# Tsession
	sk1.setBuf()
	sk1._encChal(CHc)
	wr(sk1.getBuf())

	# Rsession
	sk1.setBuf(rd(TickReqLen))
	treq = sk1._decTicketReq()
	if v2 and treq[0] == 0 :		# kenfs is fast and loose with auth formats
		treq[0] = AuthTreq;
	if treq[0] != AuthTreq :
		raise AuthError("bad server")
	CHs = treq[3]

	# request ticket from authsrv
	treq[-2],treq[-1] = user,user
	s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
	s.connect((authsrv, authport),)
	(num,CHs2,cuid,suid,Kn),stick = getTicket(s, sk1, treq)		# XXX catch
	s.close()
	if num != AuthTc or CHs != CHs2 :
		raise AuthError("bad password for %s or bad auth server" % user)
	sk1.setKn(Kn)

	# Tattach
	sk1.setBuf()
	sk1._encTattach([stick, [AuthAc, CHs, gen]])
	wr(sk1.getBuf())

	sk1.setBuf(rd(AuthLen))
	num,CHc2,gen2 = sk1._decAuth()
	if num != AuthAs or CHc2 != CHc :			# XXX check gen2 for replay
		raise AuthError("bad server")
	return


