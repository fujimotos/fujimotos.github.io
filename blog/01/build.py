from cffi import FFI

ffibuilder = FFI()

# libsodiumのヘッダ情報を読み込む
ffibuilder.set_source("sodium", """
    #include <sodium.h>
""", libraries=["sodium"])

# Python化したいCの関数の定義を記述する
ffibuilder.cdef("""
    int crypto_pwhash_str_verify(const char * hash,
                                 const char * const passwd,
                                 unsigned long long passwdlen);

    int crypto_pwhash_str(char *out, const char * const passwd,
                          unsigned long long passwdlen,
                          unsigned long long opslimit,
                          size_t memlimit);
""")

if __name__ == "__main__":
    ffibuilder.compile(verbose=True)

