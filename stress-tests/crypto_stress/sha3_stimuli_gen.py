import hashlib, pbkdf2, binascii, os, secrets, random

#########################################################################################
### 	USER DEFINED

p2coffset = 0x18000
flash_addr_offset = 0x230000

#	Number of 128-bit packets transferred for encryption
# (must be the same as in test.c or greater)
n_iter = p2coffset // 64

# Size of plaintext in bytes 
plain_size = 62

filename = "data/sha3_stim_flash.slm"
###
#########################################################################################

plain_stim = []
cipher_stim = []
plain_addr = flash_addr_offset
cipher_addr = flash_addr_offset

# Encrypt and store n_iter plaintexts
#plaintext = 0x000102030405060708090a0b0c0d0e0f
for n in range(n_iter):
    #print(plaintext)
    plaintext = random.getrandbits(plain_size * 8)
    plain_b = int.to_bytes(plaintext, length=plain_size, byteorder="big")
    #print(plain_b)
    sha3 = hashlib.sha3_512()
    sha3.update(plain_b)
    ciphertext = sha3.digest()
    #print(sha3.hexdigest())[
      
    plain_size_temp = plain_size
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
    
    # range : 512 / 8
    for i in range(0, 16):
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
