クラウド型音声認識のコマンド
============================

[![Build Status](https://travis-ci.org/koturn/sr.png)](https://travis-ci.org/koturn/sr)


# 概要

クラウド型音声認識を利用するCUIツールである．

なお，このプログラムは，公開許可があった上で公開している．




# ビルド方法

UNIX/Linux，あるいはWindowsにおけるCygwinにおいて，添付しているMakefileを
利用してビルドすればよい．
srcディレクトリに移動した上で，以下のようにすること．

```sh
$ make
```

デバッグ用のバイナリを作成する場合は，以下のようにすること．

```sh
$ make DEBUG=true
```

余談だが，最近のg++なら，以下のようにすれば，かなりの最適化がかかるように
してある．
(古いg++だと未対応のオプションを用いている)

```sh
$ make OPT=true
```

また，Microsoft Visual C++のコンパイラでもコンパイル可能である．
msvc.mkはnmake向けに書かれたMakefileであり，以下のようにすれば，コンパイル
できる．

```
> nmake /f msvc.mk
```




# 使い方

### 簡単な使い方

音声が記録されたwavファイルを対象として，次のように使用する．

```sh
$ ./sr [音声認識を行うwavファイル]
```

もしくは，wavファイルが格納されたディレクトリを指定してもよい．

```sh
$ ./sr [wavファイルがいくつかはいったディレクトリ]
```

これで，wavファイルと同一ディレクトリにwavファイルの音声の認識結果が
出力される．
出力ファイル名は，

- [wavファイル名].txt

となる．


### オプション等

このプログラムのコマンドライン引数として，以下のオプションが使用可能である．

1. -h, --help
  - プログラムの使い方を表示する．
2. -l [target-language], --language [target-language]
  - 対象ディレクトり以下のwavファイルを音声認識させる




# ファイルについて

srcディレクトリにソースコードがあり，各Makefileとコンパイラの対応は以下のように
なっている．

Makefile名 | 対象コンパイラ
-----------|---------------------------------
Makefile   | g++
clang.mk   | clang(ただし，未対応)
msvc.mk    | Microsoft Visual C++コンパイラ用




# 依存ライブラリ

以下のライブラリを，このリポジトリ内に組み込んでいる．

1. CLX C++ Libraries (BSDライセンス)
  - C++のネットワークプログラミング用ライブラリ(ヘッダのみで構成されている)
  - http://sourceforge.jp/projects/clxcpp/releases/51423
2. PicoJSON (BSDライセンス)
  - JSONパーサ(ヘッダのみで構成されている)
  - https://github.com/kazuho/picojson
3. getopt(MSVCのみ) (BSDライセンス)
  - コマンドライン引数のオプション処理用のライブラリ
  - UNIX/Linux系では，標準ライブラリだが，MSVCでは非標準ライブラリ
  - https://svn.xiph.org/trunk/theora-exp/win32/compatibility/




# ビルドについての注意

### 1. インクルードパス

各Makefileに記述してあるように，依存ライブラリへのインクルードパスは，
コンパイラのオプションで通している．
Makefileを用いずにコンパイルする場合は，インクルードパスをコンパイラに与える
こと．


### 2. 環境とコンパイラについて

1. ビルドと動作に成功した環境とコンパイラ
  - Windows7    : Cygwinのg++
  - Ubuntu12.04 : g++
2. ビルドに失敗した環境とコンパイラ
  - Windows7    : Cygwinのclang
  - Windows7    : MinGWのg++
  - Ubuntu12.04 : clang
3. ビルドに成功したが，動作しない環境とコンパイラ
  - WindowsのMSVCコンパイラ(英語のみ動作可能)

これ以外の環境については，確認していない．
