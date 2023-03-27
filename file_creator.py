import struct
# word of size 128 (2^7) bytes
data = bytearray("HelloWorHelloWorHelloWorHelloWorHelloWorHelloWorHelloWorHelloWorHelloWorHelloWorHelloWorHelloWorHelloWorHelloWorHelloWorHelloWor",'utf-8')
file = open("myfile.dat","ab")
for i in range(pow(2,26)):
    file.write(data)
file.close()
