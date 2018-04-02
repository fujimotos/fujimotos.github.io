# [Python] Python/CFFIを利用してC拡張を作成する (2/2)

本記事は全2回の連載の後編です。前回の記事は[こちらから](http://www.clear-code.com/blog/2018/1/17.html)読めます。

前回の記事では[CFFIライブラリ](https://cffi.readthedocs.io/en/latest/)を使うことで、C関数のプロトタイプ宣言から
Pythonの拡張モジュールを自動的に生成できることを見ました。

この後編ではCFFIライブラリの実務的な使い方を解説したいと思います。

（以下の内容は Ubuntu 16.04 / Python 3.5.1 で動作を確認しています）

## 1. C拡張作成の基本的な流れ

本節ではCFFIでC拡張モジュールを作成する際の基本的なフローを解説します。
前回に引き続いて、暗号ライブラリのlibsodiumを題材として具体的に手順を追っていきます。

### 1.1. CFFIをインストールする

まずはCFFIをインストールします。
たいていのディストリビューションでパッケージが用意されているので、
これを利用するのが最も手っ取り早いです。

```bash
$ sudo apt install python3-cffi
```

OSのパッケージを使わない場合は、pipを使ってインストールします。

```bash
$ pip install cffi
```

あわせて、C拡張のコンパイルに必要になるので、
gccとPythonのヘッダファイルもインストールしておいてください。

```
$ sudo apt install gcc python3-dev
```

### 1.2. 対象のライブラリをインストールする

続いて、呼び出しの対象となるCライブラリをインストールします。
ここでは、共有ライブラリ本体だけではなく、ヘッダファイルも一緒にインストールします。

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

* Pythonにエクスポートする関数のプロトタイプ宣言（だけ）を渡します。
* CFFIはここで定義された各関数についてPython向けの実装を自動生成します。

`ffi.set_source()`

* cdefで渡した以外の、ビルドに必要なあらゆる情報を渡します。
* それぞれの引数はdistutilsやコードジェネレータに引き継がれます。

各メソッドの詳しい使い方は2節に譲ります。

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

これでPythonから共有ライブラリの処理を呼び出すことができるようになりました。

## 2. ライブラリの情報の渡し方

C拡張を生成する際には`cdef()`と`set_source()`の二つのインターフェイスを通じて、
必要な情報を渡すことになります。
各メソッドの取扱いには多少分かりづらい部分があるので、本節で要点を解説します。

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

ここで定義を渡さなかった関数については、ラッパ実装が生成されない点に注意してください。

#### 作業時のポイント

ここで渡したインターフェイス情報が、コードを自動生成する時のベースになるので、
関数名や入出力の型はライブラリ側の定義と厳密にマッチする必要があります。
実務的な作業としては、ライブラリ本体のヘッダファイルからプロトタイプ宣言を
一つ一つコピペしていくことになるのですが、作業にあたってはいくつかの注意点があります：

 1. この定義の中ではCの任意の文法が使えるわけではありません。

    * 例えば、`#include`はサポートされていませんし、マクロも原則として使えません。
    * これはCFFI独自の処理系によって解析されることに由来する制約です。

 2. 定義の解釈にあたって、ライブラリ本体のヘッダファイルは参照されません。

    * 従って、定義が自己完結するように配慮する必要があります。
    * 例えば、関数定義にマクロが利用されている場合、手で展開する必要があります。

特殊な記法を使えば多少は融通を聞かせることもできるのですが、実際の取扱いでは、
ライブラリの定義に準拠した、単純な関数宣言のみで構成するのが最も障害が少ないです。

補足として、この中で構造体や型を定義することもできます。
本記事では取り扱わないので、これに関心のある方は[cdef()のドキュメント](https://cffi.readthedocs.io/en/latest/cdef.html#ffi-ffibuilder-cdef-declaring-types-and-functions)を参照してください。

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

    # source_extension: 生成されるソースファイルの拡張子
    # 具体的な利用例としては、C++の拡張を生成する場合に'.cpp'を指定します。
    source_extension='.c',

    # libraries: リンカに渡されるライブラリ情報
    # 以下の例ではリンカの実行オプションに`-lsodium`を追加しています。
    # この指定を省くとインポート時に未定義シンボルエラーが発生します。
    libraries=["sodium"]
)
```

このメソッドは実質的には「distutils/setuptoolsのプロキシ」という性格が強いです。
実際、キーワード引数はdistutilsにそのまま引き継がれるので、ビルドの細かい制御を行いたい場合は
[distutilsのリファレンス](https://docs.python.org/3.7/distutils/apiref.html#distutils.core.Extension)
を参照して引数を調整してください。


## 3. より複雑な関数に対応する

CとPythonは基本的なセマンティクスが異なっているので、
単純にCの関数をPythonに機械的にエクスポートしただけでは、扱いづらい場合が少なくありません。

例えば、ランダムなバイト列を生成する`randombytes_buf()`関数を考えてみましょう。

```c
void randombytes_buf(void * const buf, const size_t size);
```

これまでの解説を用いれば、この関数をエクスポートすること自体は容易にできます。
問題は、具体的にどのようにこの関数をPythonからコールするかです。
例えば、単純にPythonのオブジェクトを引数に与えると、
Pythonオブジェクトの内部構造を上書きしてしまい、予期しない動作を引き起こします。

```python
>>> from _sodium import ffi, lib
>>> buf = b'x' * 64
>>> lib.randombytes_buf(buf, 64)  # ???
```

この問題を解決するには、CFFIの`ffi`というインターフェイスを利用する必要があります。

### 3.1. FFIインターフェイス

`ffi`インターフェイスの役割は、Python上でC言語のセマンティクスを部分的に再現することです。
提供されている主要なメソッドを以下に示します：

|   名称           |  機能
| ---------------- | ------------------------------------------------
| `ffi.new()`      |  指定した型のメモリ領域を確保する
| `ffi.cast()`     |  型変換（キャスト）を行う
| `ffi.sizeof()`   |  データ型のサイズを取得する
| `ffi.memmove()`  |  メモリ領域をコピーする
| `ffi.string()`   |  確保したメモリ領域をPythonのバイト列に変換する

メソッドの一覧は[リファレンスマニュアル](https://cffi.readthedocs.io/en/latest/ref.html#ffi-interface)を参照してください。

具体的な利用例を以下に示します：

```python
>>> from _sodium import ffi, lib
>>> buf = ffi.new('char[]', 64)   # メモリ領域を確保する
>>> lib.randombytes_buf(buf, 64)  # 関数に引き渡す
0
>>> ffi.string(buf, 64)           # バッファをPythonのバイト列に変換する
b'\x1d\xedw+\xf9}\x8d!\xa3...'
```

Cのポインタ操作と同等の処理がPythonでできるようになっている事が見て取れると思います。

### 3.2. メモリ管理について

確保したメモリ領域はPython上では`cdata`というオブジェクトとして表現されます。

```python
>>> buf = ffi.new('char[]', 64)
>>> print(buf)
<cdata 'char[]' owning 10 bytes>
```

重要なポイントとして、確保したメモリ領域は、
対応する`cdata`オブジェクトのライフサイクルに紐付けて自動的に管理されます。
オブジェクトがGCによって回収されるとメモリ領域も解放されるので、
C言語のようにいちいち開発者の側でfreeする必要がなくなっています。

```python
# Pythonオブジェクトが回収されると、確保したメモリ領域も自動的に解放される。
# 例えば、次の関数は`intp`を明示的に解放していないが、メモリリークは起きない。
del foo():
    intp = ffi.new('int *')
    return lib.somefunc(intp)
```

### 3.3. モジュール作成のヒント

モジュールを使うために毎回`ffi`インターフェイスを操作する必要があるのは非常に面倒です。
そのため、CFFIでC拡張モジュールを作成する時は、一緒にPythonのラッパ実装を作成しておくと、
Pythonモジュールとしての使い勝手がぐんと向上します。

一般的に使われるテクニックは、C拡張のモジュールを`_`付きの名前で生成しておいて、
その上にPythonの実装をかぶせるという方式です。

```
_mymodule ... CFFIで生成した素のC拡張モジュール
mymodule  ... Pythonで作成したラッパモジュール
```

前節のコードを例にとると、次のようなラッパ実装を sodium.py という名前で保存します。

```python
from _sodium import ffi,lib

def get_randombytes(size):
    buf = ffi.new('char[]', size)
    lib.randombytes_buf(buf, size)
    return ffi.string(buf)
```

これでモジュールの利用者は`ffi`インターフェイスを意識せずに、
ライブラリの機能を利用できるようになります。

```python
>>> from sodium import get_randombytes
>>> get_randombytes(10)
b'\x93\x13\xf9z\xaaE\xf8gb\x01'
```

## 4. まとめ

本記事では、実務面に焦点をあててCFFIライブラリの使い方を解説しました。
PythonからCの共有ライブラリを扱う場合の参考になりましたら幸いです。
