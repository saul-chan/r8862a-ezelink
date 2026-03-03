#!/usr/bin/python3


print("============ start ============\n")
#read txt method one
f = open("./agcram.hex")
f_out = open("./agcram.hex.bin", 'wb+');
print("open file")
line = f.readline()
while line:
    #print(line)
    hex_bytes_value = bytes.fromhex(line)
    #print("hex_bytes_value = ",hex_bytes_value)
    #print("hex_bytes_value type = ",type(hex_bytes_value))
    int_value=hex_bytes_value[3]
    #print("int_value = ",int_value)
    #print("int_value type = ",type(int_value))
    
    byte_value=int_value.to_bytes(1,byteorder='little',signed=False)
    #print("[]byte_value = ",byte_value)
    #print("[]byte_value type = ",type(byte_value))
    f_out.write(byte_value)
    
    int_value=hex_bytes_value[2]
    #print("int_value = ",int_value)
    #print("int_value type = ",type(int_value))
    
    byte_value=int_value.to_bytes(1,byteorder='little',signed=False)
    #print("[]byte_value = ",byte_value)
    #print("[]byte_value type = ",type(byte_value))
    f_out.write(byte_value)
    
    int_value=hex_bytes_value[1]
    #print("int_value = ",int_value)
    #print("int_value type = ",type(int_value))
    
    byte_value=int_value.to_bytes(1,byteorder='little',signed=False)
    #print("[]byte_value = ",byte_value)
    #print("[]byte_value type = ",type(byte_value))
    f_out.write(byte_value)
    
    int_value=hex_bytes_value[0]
    #print("int_value = ",int_value)
    #print("int_value type = ",type(int_value))
    
    byte_value=int_value.to_bytes(1,byteorder='little',signed=False)
    #print("[]byte_value = ",byte_value)
    #print("[]byte_value type = ",type(byte_value))
    f_out.write(byte_value)
    line = f.readline()

f.close()
f_out.close()

print("============ done ============\n")
