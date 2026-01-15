import pyaes, pbkdf2, binascii, os, secrets, random

#########################################################################################
### 	USER DEFINED

filename = "data/aes_stim_flash.slm"

flash_addr_offset = 0x010000
p2coffset = 0x018000

CTR_256 = 1
CTR_192 = 1
CTR_128 = 1
CBC_256 = 1
CBC_192 = 1
CBC_128 = 1

#	Number of 128-bit packets transferred for encryption
n_iter = p2coffset // 16
#n_iter = 200

# 256-bit AES encryption key
key_256 = b'\xab\xcd\xef\x01\xab\xcd\xef\x01\xab\xcd\xef\x01\xab\xcd\xef\x01\xab\xcd\xef\x01\xab\xcd\xef\x01\xab\xcd\xef\x01\xab\xcd\xef\x01'

# 192-bit AES encryption key
key_192 = b'\xab\xcd\xef\x01\xab\xcd\xef\x01\xab\xcd\xef\x01\xab\xcd\xef\x01\xab\xcd\xef\x01\xab\xcd\xef\x01'

# 128-bit AES encryption key
key_128 = b'\xab\xcd\xef\x01\xab\xcd\xef\x01\xab\xcd\xef\x01\xab\xcd\xef\x01'

# Inicialization vector or Nonce
iv = b'\xab\xcd\xef\x01\xab\xcd\xef\x01\x00\x00\x00\x00\x00\x00\x00\x00'

###
#########################################################################################

open(filename, 'w').close()

#plaintext = 0x00000003000000020000000100000000

# Encrypt and store n_iter plaintexts
def encrypt(idx):
    idx_st = idx
    plain_stim = []
    cipher_stim = []
    for n in range(n_iter):
        plaintext = random.getrandbits(128)
        plain_b = int.to_bytes(plaintext, length=16, byteorder="big")
        ciphertext = aes.encrypt(plain_b)

        for i in range(15, -1, -1):
            plain_stim.append('@' f'{idx:06x} {plain_b[i]:02x}')
            cipher_stim.append('@' f'{idx+p2coffset:06x} {ciphertext[i]:02x}')
            idx += 1
            
    while idx < (idx_st + p2coffset):
        plain_stim.append('@' f'{idx:06x} 00')
        cipher_stim.append('@' f'{idx+p2coffset:06x} 00')
        idx += 1

    with open(filename, 'a') as f:
        plain_stim = '\n'.join(plain_stim)
        f.write(plain_stim)
        f.write('\n')
        cipher_stim = '\n'.join(cipher_stim)
        f.write(cipher_stim)
        f.write('\n')
     
if (CTR_256):
    idx = 0 + flash_addr_offset
    key = key_256
    counter = pyaes.Counter(initial_value = int.from_bytes(iv, byteorder="big"))
    aes = pyaes.AESModeOfOperationCTR(key, counter = counter)
    encrypt(idx)

if (CTR_192):
    idx = 2 * p2coffset + flash_addr_offset
    key = key_192
    counter = pyaes.Counter(initial_value = int.from_bytes(iv, byteorder="big"))
    aes = pyaes.AESModeOfOperationCTR(key, counter = counter)
    encrypt(idx)

if (CTR_128):
    idx = 4 * p2coffset + flash_addr_offset
    key = key_128
    counter = pyaes.Counter(initial_value = int.from_bytes(iv, byteorder="big"))
    aes = pyaes.AESModeOfOperationCTR(key, counter = counter)
    encrypt(idx)

if (CBC_256):
    idx = 6 * p2coffset + flash_addr_offset
    key = key_256
    aes = pyaes.AESModeOfOperationCBC(key, iv = iv)
    encrypt(idx)

if (CBC_192):
    idx = 8 * p2coffset + flash_addr_offset
    key = key_192
    aes = pyaes.AESModeOfOperationCBC(key, iv = iv)
    encrypt(idx)

if (CBC_128):
    idx = 10 * p2coffset + flash_addr_offset
    key = key_128
    aes = pyaes.AESModeOfOperationCBC(key, iv = iv)
    encrypt(idx)

