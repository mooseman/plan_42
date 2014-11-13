

# plan42.py 
# This code is released to the public domain.

import time
     

class file():
   def create(self, fname, owner):
      self.fname = name
      self.owner = owner
      self.type = ''
      self.permissions = ''


class os():
   def __init__(self, username, pw):
      self.username = username
      self.pw = pw
# Do some stuff to imitate setup during boot
      print "Welcome to Plan 42! Share and enjoy.... "
      time.sleep(3)
      print "Mounting drives... please wait..."
      time.sleep(3)
      print "Mounted / /dev /usr /etc "

      ''' Set up filesystem '''
      self.filesystem = {}
      ''' Add a few dirs to the filesystem '''
      self.filesystem['/'] = ["foo.txt", "bar.png", "baz.html"]
      self.filesystem['/usr'] = ["even.txt", "more.txt", "stuff.txt"]
      self.filesystem['/bin'] = ["cp", "mv", "rm", "mkdir", "rmdir"]
      print self.filesystem
      time.sleep(3)
      print "Done!"

   ''' A few commands. We will flesh these out later. '''
   def cp(self, source, target):
      '''self.source = source
self.target = target '''

   def ls(self, dir):
      pass
      ''' print [fname for fname in dir] '''

   def rm(self, target):
      ''' self.filesystem.del(target) '''

   def mv(self, source, target):
      ''' self.source = source
self.target = target '''


# Run the class
a = os("fred", "foobar")



