import hashlib, pbkdf2, binascii, os, secrets, random

#########################################################################################
### 	USER DEFINED

p2coffset = 0x18000
flash_addr_offset = 0x200000

#	Number of 128-bit packets transferred for encryption
# (must be the same as in test.c or greater)
n_iter = p2coffset // 64

# Size of plaintext in bytes 
plain_size = 54

filename = "data/sha2_stim_flash.slm"
###
#########################################################################################

plain_stim = []
cipher_stim = []
plain_addr = flash_addr_offset
cipher_addr = flash_addr_offset

# Encrypt and store n_iter plaintexts
#plaintext = 0x48656c6c6f
for n in range(n_iter):
    plaintext = random.getrandbits(plain_size * 8)
    #print(hex(plaintext))
    plain_b = int.to_bytes(plaintext, length=plain_size, byteorder="big")
    #print(binascii.hexlify(plain_b))
    sha2 = hashlib.sha256()
    sha2.update(plain_b)
    ciphertext = sha2.digest()
    #print(sha2.hexdigest())
      
    #plain_size_temp = plain_size
    plain_size_temp = 64
    plain_b += int.to_bytes(0x80, length=1, byteorder='big')
    plain_b += int.to_bytes(0x0, length=64 - 1 - 8 - plain_size, byteorder='big')
    plain_b += int.to_bytes(plain_size * 8, length=8, byteorder='big')
    idx = 0
    while plain_size_temp > 0:
        if plain_size_temp >= 4:
            for j in range (0, 4):
                plain_stim.append('@' f'{plain_addr:06x} {plain_b[idx*4 + 3 - j]:02x}')
                plain_addr += 1
            idx += 1
            plain_size_temp -= 4
        else:
            for j in range (0, 4 - plain_size_temp):
                plain_stim.append('@' f'{plain_addr:06x} 00')
                plain_addr += 1
            for j in range (0, plain_size_temp):
                plain_stim.append('@' f'{plain_addr:06x} {plain_b[idx*4 + plain_size_temp - 1 - j]:02x}')
                plain_addr += 1
            plain_size_temp = 0 
        
    
    # range : 256 / 8
    for i in range(0, 8):
        for j in range(0, 4):
            cipher_stim.append('@' f'{(cipher_addr + p2coffset):06x} {ciphertext[i*4 + 3 - j]:02x}')
            cipher_addr += 1

while plain_addr < p2coffset + flash_addr_offset:
    plain_stim.append('@' f'{plain_addr:06x} 00')
    plain_addr += 1
    
with open(filename, 'w') as f:
    plain_stim = '\n'.join(plain_stim)
    f.write(plain_stim)
    f.write('\n')
    cipher_stim = '\n'.join(cipher_stim)
    f.write(cipher_stim)
    f.write('\n')
