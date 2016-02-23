import sysv_ipc

key = 6666
size = 160

shm = sysv_ipc.SharedMemory(key)

memory = shm.read(size)
print ":".join("{:02x}".format(ord(c)) for c in memory)