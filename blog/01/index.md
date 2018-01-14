# Python/CFFIを利用してC拡張を作成する [1/2]

クリアコードの藤本です。皆さん、Pythonでプログラムを書いていますか？

本記事から始まる前後編2回の連載では、PythonのCFFIライブラリ — Cで実装された処理を
Pythonから呼び出すための仕組み — について解説したいと思います。

ここで "Cで実装された処理を呼び出す" とは、例えば `/usr/lib/libc.so` に
格納されているルーチンを、Pythonプログラムからコールすることを指しています。
前編にあたる本記事では、
まず最初にCFFIライブラリの基本的な仕組みを簡単に説明した上で、
実際にこのライブラリを動かしてみたいと思います。

## CFFIはどのような問題を解決するのか

Pythonで会員制のWebサービスを開発しているとしましょう。

ユーザーの認証処理をセキュアに実装するために、暗号ライブラリ libsodium
のパスワードハッシュ関数を使いたいと考えたとします。
この暗号ライブラリは、もともと C(++) 向けに提供されているものなので、
何とか工夫してPythonから必要な関数を呼び出せるようにする必要があります。
どうすれば、これを実現できるでしょうか？

この問題に対する伝統的な戦略は、CPythonが提供している C/API を利用して、
拡張モジュールを作成することです。具体的には次のようにヘッダ定義を読み込んで、
ラッパ関数を一つ一つ作成していくことになります：

```c
#include <Python.h>
#include <sodium.h>

/*
 * PythonのC/APIで crypto_pwhash_str_verify() 関数をラップする
 */
static PyObject* sodium_pwhash_verify(PyObject *self, PyObject *args)
{
    const char *hash, *password;
    Py_ssize_t len;
    int res;

    // 引数をCの型に変換する (Python -> C)
    if (!PyArg_ParseTuple(args, "bbn", &hash, &password, &len))
        return NULL;

    // 対象の関数をコールする
    res = crypto_pwhash_str_verify(hash, password, len);

    // 返り値をPythonのオブジェクトに変換する (C -> Python)
    return PyLong_FromLong(res);
}

// 以下、他の関数も定義する...
```

この戦略は20年以上に渡って用いられており、
2018年現在も多くのPythonモジュールがこの方式を採用しています。
したがって、この方式が現実に（それもかなり有効に）機能することにはほとんど異論がありません。
一方で、この方式で拡張モジュールを作成するのには、
それなりの背景知識が要求されるというのもまた事実です。
およそC/APIは実行系／ランタイムの内部実装と表裏一体なので、
CPythonに特有の仕組みや細かい決まりごと（たとえば参照カウントの扱い）を
おさえておく必要があるからです。

CFFIはこのようなPythonとCの間の橋渡しを楽にするために開発されたFFI
(Foreign Function Interface) モジュールです。
LuaJitのFFI実装を参考に、2012年に開発がスタートしました。
2015年にメジャーバージョンの1.X系がリリースされており、既に2Dグラフィック処理のcairoや、
サウンドサーバーのJACKとの連携などに幅広く利用されています。

1. https://download.libsodium.org/doc/
2. https://github.com/Kozea/cairocffi
3. https://github.com/spatialaudio/jackclient-python/

## CFFIの基本的な仕組み

最初の例に立ち戻りましょう。

そもそも、Cで提供される関数は（原則として）入出力が型によってきっちり定義されるので、
Pythonとの結合部分はある程度自動で生成できるんじゃないか、というのはごく自然な発想です。

例えば、`int foo(const char *)`という関数定義が与えられたとすると、
引数の `char*` 型をPythonのバイト列に、返り値の `int` 型をPythonの整数オブジェクトに対応させるのは、
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

CFFIとlibsodiumをインストールした上で、このスクリプトを実行します。
すると、builderが関数の定義情報からPython向けのラッパ実装を自動的に生成して、
ソースコードのコンパイルまでしてくれます：

```bash
# 必要なライブラリのインストール (Ubuntu/Debian)
$ sudo apt install python3-cffi libsodium-dev

# C拡張コード生成 + コンパイル
$ python3 build.py
$ ls
build.py sodium.c sodium.cpython-35m-x86_64-linux-gnu.so sodium.o
```

この`sodium.c`が自動生成されたC拡張のソースコードで、
`sodium.*.so`というのがコンパイル済みのモジュールです。
このバイナリモジュールはそのままPythonから呼び出すことができます。
試しに、適当な入力を与えてみましょう：

```python
>>> from sodium import lib
>>> passwd = b'secret'
>>> pwhash = (b'$argon2i$v=19$m=131072,t=6,p=1$r/g1+z50+W9RWUMRy4xu+g$'
...           b'BNWoK+o6Hlcu98scoCxlNrYGo8hacShQ2nkc4RS5wZk')
>>> lib.crypto_pwhash_str_verify(pwhash, passwd, len(correct))
0
```

この例で、C言語の型とPythonのオブジェクトが透過的に接続されているのが
確認いただけると思います。

### 残された課題

ここまでは「Cの関数定義から拡張を自動生成する」という観点からCFFIを特徴付けてきました。

しかし、実のところこの発想そのものはそれほど目新しいものではありません。
例えば、1990年代後半から利用されているコードジェネレータとしてSWIG[1]があり、
CFFIと同様に、Cのヘッダ定義からPythonの拡張モジュールを自動生成することができます。

ではなぜCFFIライブラリなのか？ ここで重要となる一つのポイントは、
「PythonとCの間に自然な対応関係が存在しないケースをどう扱うか」です。

この問題の解決は後編に回したいと思います。

1. http://swig.org/
