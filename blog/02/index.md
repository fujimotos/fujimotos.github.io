# [Python] Python/CFFIを利用してC拡張を作成する (2/2)

本記事は全2回の連載の後編です。前回の記事は[こちらから](http://www.clear-code.com/blog/2018/1/17.html)読めます。

前回の記事では CFFIライブラリを使うことで、C関数のプロトタイプ宣言から
Pythonの拡張モジュールを自動的に生成できることを見ました。

この後編ではCFFIライブラリの実務的な使い方を解説したいと思います。

## 1. CFFIのインストール方法

CFFIはpip経由でインストールできます：

```bash
$ pip install cffi
```

主要なLinuxディストリビューションについては配布パッケージが提供されているので、
そちらからインストールすることもできます：

```bash
# Debian/Ubuntu
$ sudo apt install python-cffi
# CentOS/RedHat
$ sudo yum install python-cffi
```

## 2. C拡張を生成する（基本編）

### 2.1. 対象のライブラリをインストールする

CFFIで拡張モジュールを生成する場合、共有ライブラリ本体に加えて、
ライブラリのヘッダファイルもインストールしておく必要があります。
この二つは別々にパッケージングされている事が多いので、
事前に必ず両方インストールしておくようにしてください。

例えば、libsodiumであれば次の二つのパッケージをインストールします：

```bash
$ sudo apt install libsodium18 libsodium-dev
# - libsodium18   ... 共有ライブラリ本体 (libsodium.so)
# - libsodium-dev ... ヘッダファイル (libsodium.h)
```

他のライブラリでも要領は同じです。

### 2.2. 定義する


## 2.3. C拡張を生成する（応用的な例）

ffiというオブジェクトが存在していて、これを使うと部分的にCのセマンティクスを模倣できるようになってます。
