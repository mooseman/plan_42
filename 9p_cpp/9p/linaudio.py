#!/usr/bin/python
"""
A 9P interface to linux's audio device.
XXX This uses the OSS interface.  Should we be using ALSA?
"""

import os
import fcntl
import array
import time

import srv

ServError = srv.ServError

if 1 :		# device definitions
	PATH = "/dev/dsp"

	_IOC_NONE = 0
	_IOC_WRITE = 0x40000000
	_IOC_READ = -2147483648

	def _IOC(dir, type, nr, sz) :
		return dir | (sz<<16) | (ord(type)<<8) | nr
	def _IO(type, nr) :
		return _IOC(_IOC_NONE, type, nr, 0)
	def _IOWR(type, nr, sz) :
		return _IOC(_IOC_WRITE|_IOC_READ, type, nr, sz)

	SNDCTL_DSP_SYNC = _IO('P', 1)
	SNDCTL_DSP_SPEED = _IOWR('P', 2, 4)
	SNDCTL_DSP_GETBLKSIZE = _IOWR('P', 4, 4)
	SNDCTL_DSP_SETFMT = _IOWR('P', 5, 4)
	SNDCTL_DSP_CHANNELS = _IOWR('P', 6, 4)

	AFMT_S16_LE = 0x10

class AudioFs(object) :
	dirs = { '/' : ['audio', 'volume', 'audiostat'] }
	cancreate = 0
	type = ord('A')
	def __init__(self, path=None) :
		if path is None :
			path = PATH
		self.fd = srv._os(os.open, path, os.O_RDWR)
		self.src = "mic"
		self.vol = 75
		self._setSpeed(44100)
		self._setFmt(AFMT_S16_LE)
		self._setChannels(2)

		self.busy = None

	def _ioctl(self, code, n=0) :
		args = array.array('i', [n])
		srv._os(fcntl.ioctl, self.fd, code, args)
		return args[0]
	def _sync(self) :
		return self._ioctl(SNDCTL_DSP_SYNC, 0)
	def _setSpeed(self, n) :
		return self._ioctl(SNDCTL_DSP_SPEED, n)
	def _getBlksize(self) :
		return self._ioctl(SNDCTL_DSP_GETBLKSIZE)
	def _setChannels(self, n) :
		return self._ioctl(SNDCTL_DSP_CHANNELS, n)
	def _setFmt(self, n) :
		return self._ioctl(SNDCTL_DSP_SETFMT, n)

	def estab(self, f, isroot) :
		f.autype = None
		if isroot :
			f.autype = '/'
			f.isdir = 1
		else :
			pt = f.parent.autype
			if (pt in self.dirs) and (f.basename in self.dirs[pt]) :
				f.autype = f.basename

	def _illegal(self, *args) :
		raise ServError("illegal")
	remove = _illegal
	wstat = _illegal

	def list(self, f) :
		return self.dirs[f.autype]

	def stat(self, f) :
		now = int(time.time())
		return (0, 0, 0, None, 0644, now, now, 1024, None, 'uid', 'gid', 'muid')
	def exists(self, f) :
		return (f.autype is not None)
	def walk(self, f, fn, n) :
		if self.exists(fn) :
			return fn
	def open(self, f, mode) :
		if f.autype == 'audio' :
			if self.busy :
				raise ServError("busy")
			self._sync()
			self.busy = f
	def clunk(self, f) :
		if f.autype == 'audio' :
			if self.busy is f :
				self.busy = None

	def read(self, f, pos, l) :
		if f.autype == 'audio' :
			return srv._os(os.read, self.fd, l)
		elif f.autype == 'volume' :
			buf = "%s %d\n" % (self.src, self.vol)
		elif f.autype == 'audiostat' :
			# XXX
			buf = "bufsize %6d buffered %6d offset %10d time %19d\n" % (1024, 0, 0, 0)
		return buf[pos : pos+l]
	def write(self, f, pos, buf) :
		if f.autype == 'audio' :
			srv._os(os.write, self.fd, buf)
		elif f.autype == 'volume' :
			pass # XXX
		elif f.autype == 'audiostat' :
			pass # XXX
		return len(buf)

root = AudioFs()
mountpoint = '/'

