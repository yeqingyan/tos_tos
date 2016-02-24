import sysv_ipc
import struct

key = 6666
size = 160

shm = sysv_ipc.SharedMemory(key)

memory = shm.read(size)
print len(memory)
'''
print "Pin Level 1C"
print ":".join("{:02x}".format(ord(c)) for c in memory[28:32])

print "Pin Level 20"
print ":".join("{:02x}".format(ord(c)) for c in memory[32:36])

print "Pin Level 28"
print ":".join("{:02x}".format(ord(c)) for c in memory[40:44])

print "Pin Level 2c"
print ":".join("{:02x}".format(ord(c)) for c in memory[44:48])
'''
print "Pin Level 34"
print ":".join("{:02x}".format(ord(c)) for c in memory[52:56])

print "Pin Level 38"
print ":".join("{:02x}".format(ord(c)) for c in memory[56:60])

'''
for i in range(0,16):
    print "{0:2x}-{1:2x}".format(i*10, (i+1)*10) + "\t" + ":".join("{:02x}".format(ord(c)) for c in memory[i*10:(i+1)*10])
'''
'''
channel = 14
offset = 52 + (channel/32*4)
mask = 1 << (channel % 32)
value = struct.unpack('<i', shm.read(4, offset))
print "{0}".format(value)
value = value[0] & mask
print "{0:x}".format(value)
print "{0:x}".format(mask)
'''