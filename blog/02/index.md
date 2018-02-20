# [Python] Python/CFFIを利用してC拡張を作成する (2/2)

本記事は全2回の連載の後編です。前回の記事は[こちらから](http://www.clear-code.com/blog/2018/1/17.html)読めます。

前回の記事では CFFIライブラリを使うことで、C関数のプロトタイプ宣言から
Pythonの拡張モジュールを自動的に生成できることを見ました。

この後編ではCFFIライブラリの実務的な使い方を解説したいと思います。

なお以下はDebian 9.2とFedora 27で動作を確認しています。

## 1. CFFIをインストールする

CFFIについては大抵のディストリビューションでパッケージが配布されています：

```bash
# Debian/Fedora共通
$ sudo apt install python3-cffi
```

またpip経由でインストールすることもできます：

```bash
$ pip install cffi
```

## 2. C拡張を生成する（基本編）

### 2.1. ライブラリをインストールする

CFFIで拡張モジュールを生成する場合、共有ライブラリ本体に加えて、
ライブラリのヘッダファイルもインストールしておく必要があります。
この二つは普通別々にパッケージングされているので、
事前に必ず両方インストールしておくようにしてください。

例えば、lzma形式の圧縮ファイル (`.xz`) を扱う場合は次のようになります：

```bash
# Debian
# - liblzma5    ... 共有ライブラリ本体 (liblzma.so)
# - liblzma-dev ... ヘッダファイル (lzma.h)
$ sudo apt install libsodium18 libsodium-dev

# Fedora
# - xz-libs  ... 共有ファイル本体 (liblzma.so)
# - xz-devel ... ヘッダファイル (lzma.h)
$ sudo apt install libsodium libsodium-devel
```

C拡張をコンパイルする際に必要になるので、
あわせてPythonのヘッダファイルもインストールしておきます：

```
# Debian/Fedora
$ sudo apt install python3-dev
```

### 2.2. 関数を定義する

単純な関数であれば特に
次のスクリプトを `builder.py` という名前で保存します：

```python
from cffi import FFI

ffibuilder = FFI()

ffibuilder.set_source("lzma", """
    #include <lzma.h>
""", libraries=["lzma"])

ffibuilder.cdef("""
    uint32_t lzma_version_number(void);
""")

if __name__ == "__main__":
    ffibuilder.compile(verbose=True)
```

これを実効する

### 2.3. どのように動いているのか？

```python
from cffi import FFI

ffibuilder = FFI()

ffibuilder.set_source("lzma", """
    #include <lzma.h>
""", libraries=["lzma"])

ffibuilder.cdef("""
    uint32_t lzma_version_number(void);
""")

if __name__ == "__main__":
    ffibuilder.compile(verbose=True)
```




## 2.3. C拡張を生成する（応用的な例）

ffiというオブジェクトが存在していて、これを使うと部分的にCのセマンティクスを模倣できるようになってます。
