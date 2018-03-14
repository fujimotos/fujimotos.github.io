# [Python] Python/CFFIを利用してC拡張を作成する (2/2)

本記事は全2回の連載の後編です。前回の記事は[こちらから](http://www.clear-code.com/blog/2018/1/17.html)読めます。

前回の記事ではCFFIライブラリを使うことで、C関数のプロトタイプ宣言から
Pythonの拡張モジュールを自動的に生成できることを見ました。

この後編ではCFFIライブラリの実務的な使い方を解説したいと思います。

（以下の内容は Ubuntu 16.04 / Python 3.5.1 で動作を確認しています）

## 1. C拡張作成の基本的な流れ

本章ではCFFIでC拡張モジュールを作成する際の基本的なフローを解説します。
前回に引き続いて、暗号ライブラリのlibsodiumを題材として具体的に手順を追っていきます。

### 1.1. CFFIをインストールする

CFFIライブラリについてはAPTパッケージが用意されています。

```bash
$ sudo apt install python3-cffi
```

gccとPythonのヘッダファイルもインストールしておきます。

```
$ sudo apt install gcc python3-dev
```

### 1.2. 対象のライブラリをインストールする

まずは呼び出しの対象となる共有ライブラリをインストールします。
コンパイルのステップで必要になるので、共有ライブラリ本体だけではなく、
ヘッダファイルも一緒にインストールします。

例えば、libsodiumの場合は次のようにインストールします：

```bash
# 次のパッケージをインストールする
# - libsodium18   ... 共有ライブラリ本体 (libsodium.so)
# - libsodium-dev ... ヘッダファイル (sodium.h)
$ sudo apt install libsodium18 libsodium-dev
```

他のライブラリも多くのケースで同様のパッケージングがされているので、
利用したいライブラリを`apt search`コマンドで探してみてください。

### 1.3. CFFIにライブラリの情報を渡す

以下の内容を builder.py という名前で保存してください。

```python
from cffi import FFI

ffibuilder = FFI()

ffibuilder.cdef("""
    size_t crypto_stream_keybytes(void);
    size_t crypto_stream_noncebytes(void);
""")

ffibuilder.set_source("_sodium", """
    #include <sodium.h>
""", libraries=["sodium"])

if __name__ == "__main__":
    ffibuilder.compile(verbose=True)
```

このスクリプトはlibsodium前提の内容になってますが、
`cdef()`と`set_source()`の二つのメソッドの呼び出しを書き換えることで、
他のライブラリに応用することができます。
この二つのメソッドの違いは一見すると分かりにくいのですが、
次のような住み分けがなされています。

`ffi.cdef()`

* Pythonにエクスポートする関数のプロトタイプ宣言を渡す。
* ここで渡した情報はCFFIの独自の処理系によって解釈される。

`ffi.set_source()`

* cdefで渡した以外の、ビルドに必要なあらゆる情報を渡す。
* それぞれの引数はdistutilsやコードジェネレータに引き継がれる。

各メソッドの詳しい使い方は2章に譲ります。

### 1.4. C拡張モジュールを生成する

定義したスクリプトをPythonに渡すと一連のビルド処理が走り、C拡張が生成されます。

```bash
$ python3 builder.py
```

カレントディレクトリにC拡張モジュール (`*.so`) が生成されていることが確認できたら、
実際にPythonから処理を呼び出してみましょう。

```python
>>> from _sodium import ffi, lib
>>> lib.crypto_stream_keybytes()
32
>>> lib.crypto_stream_noncebytes()
24
```

## 2. ライブラリの情報の渡し方

C拡張を生成する際には`cdef()`と`set_source()`の二つのインターフェイスを通じて、
必要な情報を渡すことになります。
各メソッドの取扱いには多少分かりづらい部分があるので、本章で要点を解説します。

### 2.1. cdef: 関数のプロトタイプ宣言を渡す

`cdef()`メソッドには、Pythonにエクスポートしたい関数のプロトタイプ宣言を渡します。

```python
# 例: libsodiumのストリーム暗号処理をエクスポートする
ffibuilder.cdef("""
  size_t crypto_stream_keybytes(void);
  size_t crypto_stream_noncebytes(void);
  int crypto_stream(unsigned char *c, unsigned long long clen,
                    const unsigned char *n, const unsigned char *k);
""")
```

ここで定義を渡さなかった関数については、Pythonから呼び出すことができない点に注意してください。

実務的な作業としては、ライブラリ本体のヘッダファイルからプロトタイプ宣言を一つ一つ
コピペしていくことになるのですが、作業にあたってはいくつかの注意点があります：

 1. この定義の中ではCの任意の文法が使えるわけではありません。

    * 例えば、`#include`はサポートされていません。
    * また、マクロもごく限られたケース以外は使えません。

 2. 定義の解釈にあたって、ライブラリ本体のヘッダファイルは参照されません。

    * 従って、定義が自己完結するように配慮する必要があります。
    * 例えば、関数定義にマクロが利用されている場合、前もって手で展開する必要があります。

 3. 関数のインターフェイスはライブラリ側の定義とマッチする必要があります。

    * 例えばlongのポインタ型の引数をintのポインタ型として宣言する事は許されてません。

特殊な記法を使えば多少は融通を聞かせることもできるのですが、実際の取扱いでは、
ライブラリの定義に準拠した、単純な関数宣言のみで構成するのが最も障害が少ないです。

補足として、この中で構造体や型を定義することもできます。
本記事では取り扱わないので、これに関心のある方は[cffiのこちらのドキュメント](https://cffi.readthedocs.io/en/latest/cdef.html#ffi-ffibuilder-cdef-declaring-types-and-functions)を読んでください。

### 2.2. set\_source: ビルドに必要な情報を渡す

`set_source()`メソッドにはビルドに必要なその他の情報を渡します。
名前からは非常に分かりづらいのですが、このメソッドを通じてコード生成からコンパイル
までの一連のフローを制御できる設計になっています。

以下に主要な引数についてインラインで解説を加えます：

```python
ffibuilder.set_source(
    # module_name: 生成されるC拡張モジュールの名称
    # 例えば'foo'とすると`import foo`でインポート可能になります。
    module_name="_sodium",

    # source: 自動生成時に埋め込むコード
    # この引数を通じて任意のC言語の処理を自動生成コードに埋め込むことができます。
    # （ただ現実の大半のケースではライブラリのヘッダをincludeするだけです）
    source="""#include <sodium.h>""",

    # source_extension: 生成されるC拡張ソースファイルの拡張子
    # 具体的な利用例としては、C++の拡張を生成する場合に'.cpp'を指定します。
    source_extension='.c',

    # libraries: リンカに渡されるライブラリ情報
    # 以下の例では`-lsodium`をリンカの実行オプションに追加しています。
    # この指定をしておかないとインポート時に未定義シンボルエラーが発生します。
    libraries=["sodium"]
)
```

このメソッドは実質的には「distutils/setuptoolsのプロキシ」という性格が強いです。
キーワード引数はdistutilsにそのまま引き継がれるので、ビルドの細かい調整を行いたい場合は
[distutilsのAPIリファレンス](https://docs.python.org/3.7/distutils/apiref.html#distutils.core.Extension)を参照してください。

なお、モジュール名に`_`のprefixをつけている理由は次章で説明します。

## 3. より複雑な関数に対応する

CとPythonは基本的なセマンティクスが異なっているので、
単純にCの関数をPythonに機械的にエクスポートしただけでは、扱いづらい場合が少なくありません。

例えば、乱数を生成する`randombytes_buf()`関数を考えてみましょう。

```c
void randombytes_buf(void * const buf, const size_t size);
```

これまでの解説を用いれば、この関数をエクスポートすること自体は容易にできます。
問題は、具体的にどのようにこの関数をPythonからコールするかです。
例えば、単純にPythonのオブジェクトを引数に与えると、
Pythonの管理するメモリ領域を上書きしてしまい、予期しない動作を引き起こします。

```python
>>> from _sodium import ffi, lib
>>> buf = b''
>>> lib.randombytes_buf(buf, 64)  # ???
Segmentation fault (core dumped)
```

この問題を解決するには、CFFIの`ffi`というインターフェイスを利用する必要があります。

### 3.1. FFIインターフェイス

`ffi`オブジェクトを利用すると、C言語のセマンティクスをPython上で再現することができます。

|   名称           |  機能
| ---------------- | ------------------------------------------------
| `ffi.new()`      |  指定した型のメモリ領域を確保する
| `ffi.cast()`     |  型変換（キャスト）を行う
| `ffi.sizeof()`   |  データ型のサイズを取得する
| `ffi.memmove()`  |  メモリ領域をコピーする
| `ffi.string()`   |  確保したメモリ領域をPythonのバイト列に変換する

提供されているメソッドの一覧は[リファレンスマニュアル](https://cffi.readthedocs.io/en/latest/ref.html#ffi-interface)を参照してください。

具体的な利用例を以下に示します：

```python
>>> from _sodium import ffi, lib
>>> buf = ffi.new('char[]', 64)   # メモリ領域を確保する
>>> lib.randombytes_buf(buf, 64)  # 関数に引き渡す
0
>>> ffi.string(buf)               # バッファをPythonのバイト列に変換する
b'\x1d\xedw+\xf9}\x8d!\xa3...'
```

なお、確保したメモリ領域はPython上では`cdata`というオブジェクトとして表現されます。
このオブジェクトについていくつか補足しておきます：

* メモリ領域はこのオブジェクトのライフサイクルに紐付けて自動管理されます。  
  例えば、オブジェクトがGCで回収されると、メモリ領域も自動的に解放されます。
* Cのポインタ同様にインデックスによる参照もサポートしています。  
  例えば、`buf[0]`とすると先頭のデータにアクセスできます。

### 3.2. モジュール作成のヒント

モジュールを使うときに毎回`ffi`インターフェイスを扱う必要があるのは非常に煩瑣です。
そこで、インターフェイス実装をかぶせると使い勝手が格段によくなります。


```
_sodium.so  ... CFFIで生成したC拡張モジュール
sodium.py   ... Pythonで作成したインターフェイスモジュール
```

例えば、先ほどの`randombytes_buf`関数を例にとると、
次のようなラッパ実装を sodium.py という名前で保存します。

```python
from _sodium import ffi,lib

def get_randombytes(size):
    buf = ffi.new('char[]', size)
    lib.randombytes_buf(buf, size)
    return ffi.string(buf)
```


```
>>> from sodium import get_randombytes
>>> get_randombytes(10)
b'\x93\x13\xf9z\xaaE\xf8gb\x01'
```


## 4. まとめ
