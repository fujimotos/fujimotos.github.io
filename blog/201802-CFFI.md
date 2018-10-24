# [Python] Python/CFFIを利用してC拡張を作成する (1/2)

クリアコードの藤本です。皆さん、Pythonでプログラムを書いていますか？

本記事から始まる前後編2回の連載では、Pythonの
[CFFIライブラリ](https://cffi.readthedocs.io/en/latest/)
— Cで実装された処理をPythonから呼び出すための仕組み —
について解説したいと思います。

ここで "Cで実装された処理を呼び出す" とは、
例えば `/usr/lib/libc.so` に格納されているCのルーチンを、
Pythonプログラムから直接コールすることを指しています。
前編にあたる本記事では、
まず最初にCFFIライブラリの大きな仕組みについて簡単な説明を加えた上で、
実際にこのライブラリを動かしてみたいと思います。

## CFFIは何を解決するのか

Pythonで会員制のWebサービスを開発しているとしましょう。

ユーザーの認証処理をセキュアに実装するために、
暗号ライブラリ [libsodium](https://download.libsodium.org/doc/)
のパスワードハッシュ関数を使いたいと考えたとします。
この暗号ライブラリは、もともと C(++) 向けに提供されているものなので、
何とか工夫してPythonから必要な関数を呼び出せるようにする必要があります。
どうすれば、これを実現できるでしょうか？

この問題に対する伝統的な戦略は、CPythonが提供しているC言語向けのAPIを利用して、
拡張モジュールを作成することです。具体的には次のようにヘッダ定義を読み込んで、
ラッパ関数を一つ一つ作成していくことになります：

```c
#include <Python.h>
#include <sodium.h>

/*
 * PythonのC/APIで crypto_pwhash_str_verify() 関数をラップする
 */
static PyObject* sodium_pwhash_str_verify(PyObject *self, PyObject *args)
{
    char *hash, *password;
    Py_ssize_t len;
    int res;

    // 引数をCの型に変換する (Python -> C)
    if (!PyArg_ParseTuple(args, "yyn", &hash, &password, &len))
        return NULL;

    // 対象の関数をコールする
    res = crypto_pwhash_str_verify(hash, password, len);

    // 返り値をPythonのオブジェクトに変換する (C -> Python)
    return PyLong_FromLong(res);
}

// 他の関数も定義してモジュールとしてエクスポートする
```

この戦略は20年以上に渡って用いられており、
2018年現在も多くのPythonモジュールがこの方式を採用しています。
したがって、この方式が現実に（それもかなり有効に）機能することにはほとんど異論がありません。

一方で、この方式で拡張モジュールを作成するのには、
それなりの背景知識が要求されるというのもまた事実です。
およそC/APIは実行系／ランタイムの内部実装と表裏一体なので、
CPythonに特有の仕組みや細かい決まりごと（たとえば参照カウントの扱いや例外をめぐる処理）
をおさえておく必要があるからです。

CFFIはこのようなPythonとCの間の橋渡しを楽にするために開発されたFFI
(Foreign Function Interface) ライブラリです。
LuaJitのFFI実装を参考に、2012年に開発がスタートしました。
2015年にメジャーバージョンの1.0.1がリリースされており、
既に2Dグラフィック処理の [cairo](https://github.com/Kozea/cairocffi) や、
サウンドサーバーの [JACK](https://github.com/spatialaudio/jackclient-python/)
との連携などに幅広く利用されています。

## CFFIはどのように問題を解決するのか

最初の例に戻りましょう。

そもそも、Cで提供される関数は（原則として）入出力が型によってきっちり定義されるので、
Pythonとの結合部分はある程度自動で生成できるんじゃないか、というのはごく自然な発想です。
例えば、`int foo(const char *)`という関数定義が与えられたとすると、
引数の `char*` をPythonのバイト列に、
返り値の `int` をPythonの整数オブジェクトに機械的に対応させるのは、
それほど難しいことではなさそうです。

CFFIライブラリはまさにこの"つなぎ"の部分の自動生成を行ってくれます。
実例で見てみましょう。まずは、次の内容を`build.py`というファイルに保存します：

```python
from cffi import FFI

ffibuilder = FFI()

# libsodiumのヘッダ情報をincludeする
ffibuilder.set_source("sodium", """
    #include <sodium.h>
""", libraries=["sodium"])

# Python化したいC関数の定義を記述する
ffibuilder.cdef("""
    int crypto_pwhash_str_verify(const char * hash,
                                 const char * const passwd,
                                 unsigned long long passwdlen);
""")

if __name__ == "__main__":
    ffibuilder.compile(verbose=True)
```

この例では、libsodiumの関数 `crypto_pwhash_str_verify()` を移植しています
（上のC/APIの例で用いたのと同じ関数です）。
`set_source()`で読み込むライブラリを指定し、
`cdef()` で関数定義を与えているのが見て取れると思います。

依存関係をインストールした上で、このスクリプトを実行しましょう。
すると、CFFIが定義情報からPython向けのラッパ実装を自動的に生成し、
コンパイルまで行ってくれます：

```bash
# 必要なライブラリのインストール (Ubuntu/Debian)
$ sudo apt install python3-cffi libsodium-dev
...
# C拡張コード生成 + コンパイル
$ python3 build.py
...
# 生成結果
$ ls
build.py sodium.c sodium.cpython-35m-x86_64-linux-gnu.so sodium.o
```

この`sodium.c`がC拡張のソースコードで、
`sodium.*.so`というのがコンパイル済みのモジュールです。

このモジュールはそのままPythonから呼び出すことができます。
試しに、適当な入力を与えてみましょう：

```python
>>> from sodium import lib
>>> passwd = b'secret'
>>> pwhash = (b'$argon2i$v=19$m=131072,t=6,p=1$r/g1+z50+W9RWUMRy4xu+g$'
...           b'BNWoK+o6Hlcu98scoCxlNrYGo8hacShQ2nkc4RS5wZk')
>>> lib.crypto_pwhash_str_verify(pwhash, passwd, len(passwd))
0
```

この例で、C言語の型とPythonのオブジェクトが透過的に接続されているのが
確認いただけると思います。

## 残された課題／後編へのつなぎ

連載の前編を締めるにあたって、大急ぎで次の二つの点を指摘しておきたいと思います。

まず第一は、このコードジェネレーティングの戦略は万能の解決策ではないということです。
前節で見たとおり、簡単な関数であれば簡単に移植できます。
しかし、真面目にライブラリを移植しはじめると、
ほぼ間違いなく自動生成だけでは対応できないケースに出くわすことになります。

この代表例がポインタです。とくに、Cのコードでよく使われるテクニックとして
「呼び出し側でデータ領域を確保して、ポインタ経由で関数に中身を書き換えてもらう」
というコードパターンがありますが、これをどうPythonのコードに置き換えるかは、
決して自明ではありません。例えば、次の関数を移植する方法を考察してみてください：

```c
// `size`バイト分のランダムデータで`buf`を埋める
void randombytes_buf(void * const buf, const size_t size);
```

この問題の解決は後編に回したいと思います。

もう一つは、CFFIライブラリの技術的な位置づけについてです。
実はここまでの「CFFI = C拡張の自動生成ライブラリ」という定式化はかなり話を端折ったものです。
この点については、他の競合する技術（Cython/ctypes/SWIG）との関係で説明する必要があるので、
後編で一節をまるまる割く予定です。
