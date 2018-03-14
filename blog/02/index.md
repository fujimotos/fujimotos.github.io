# [Python] Python/CFFIを利用してC拡張を作成する (2/2)

本記事は全2回の連載の後編です。前回の記事は[こちらから](http://www.clear-code.com/blog/2018/1/17.html)読めます。

前回の記事ではCFFIライブラリを使うことで、C関数のプロトタイプ宣言から
Pythonの拡張モジュールを自動的に生成できることを見ました。

この後編ではCFFIライブラリの実務的な使い方を解説したいと思います。

（以下の内容は Ubuntu 16.04 / Python 3.5.1 で動作を確認しています）

## 1. C拡張作成の基本的な流れ

本章ではCFFIでC拡張モジュールを作成する作業のフローを解説します。
前回に引き続いて、具体例として暗号ライブラリのlibsodiumを使って手順を追っていきます。

### 1.1. CFFIをインストールする

CFFIライブラリについてはAPTパッケージが用意されています。

```bash
$ sudo apt install python3-cffi
```

C拡張をコンパイルする際に必要になるので、
あわせてgccとPythonのヘッダファイルもインストールしておきます。

```
$ sudo apt install gcc python3-dev
```

### 1.2. 対象のライブラリをインストールする

まずは呼び出しの対象となる共有ライブラリをインストールします。
次のコンパイルのステップで必要になるので、共有ライブラリ本体だけではなく、
ヘッダファイルも一緒にインストールします。

例えば、libsodiumの場合は次のようにインストールします：

```bash
# 次のパッケージをインストールする
# - libsodium18   ... 共有ライブラリ本体 (libsodium.so)
# - libsodium-dev ... ヘッダファイル (sodium.h)
$ sudo apt install libsodium18 libsodium-dev
```

他のライブラリも多くのケースで同様のパッケージングがされているので、
Pythonから利用したいライブラリを`apt search`コマンドで探してみてください。

### 1.3. CFFIにライブラリの定義を渡す

以下の内容を builder.py という名前で保存してください。

```python
from cffi import FFI

ffibuilder = FFI()

ffibuilder.cdef("""
    int sodium_library_version_major(void);
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

* Pythonにエクスポートする関数のプロトタイプ宣言のみを受け取る。
* ここで渡した情報はCFFIのヘッダパーサによって解析される。

`ffi.set_source()`

* cdefで渡した以外の、ビルドに必要なあらゆる情報を受け取る。
* ここで渡した情報はdistutilsやコードジェネレータに取りつがれる。

各メソッドの詳しい使い方は2章に譲ります。

### 1.4. C拡張モジュールを生成する

定義したスクリプトをPythonに渡すと一連のビルド処理が走り、C拡張が生成されます。

```bash
$ python3 builder.py
```

カレントディレクトリにC拡張モジュール (`*.so`) が生成されていることが確認できたら、
次のコードでライブラリの処理を実際に呼び出してみてください。

```python
from sodium import ffi, lib
print(lib.sodium_library_version_major())  # => 9
```

## 2. ビルド定義の渡し方

前章で簡単に触れましたが、C拡張を生成する際には`cdef()`と`set_source()`の
二つのインターフェイスを通じて、必要な情報を渡すことになります。



## 3. 複雑な関数をエクスポートする

ffiというオブジェクトが存在していて、これを使うと部分的にCのセマンティクスを模倣できるようになってます。


## 4. まとめ
