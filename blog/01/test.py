from sodium import ffi, lib

out = ffi.new('char[256]');
pwd = b"secret"

lib.crypto_pwhash_str(out, pwd, len(pwd), 6, 128*1024*1024);


hash = bytes(ffi.string(out))
print(lib.crypto_pwhash_str_verify(hash, pwd, len(pwd)))

pwd = b"xxx"
print(lib.crypto_pwhash_str_verify(hash, pwd, len(pwd)))
