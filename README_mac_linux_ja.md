# Rogue 3.6 — macOS / Linux（日本語）

この文書では、**macOS、Linux、Raspberry Pi OS** でのビルド、起動、インストール、セーブ・スコアなどの保存先について説明します。Rogue 3.6とJEC版の紹介、および各OS・言語向け文書の一覧は[リポジトリのREADME](README.md)をご覧ください。操作方法や道具・オプションの説明は、同梱の[「【運命の洞窟】への案内書」](rogue_ja.html)にまとめています。

English version: [README_mac_linux_en.md](README_mac_linux_en.md)

## 1. 開発環境の準備

ソースコードを展開し、ターミナルでそのフォルダを開いてください。

**macOS：** Appleのコマンドラインツールをインストールします。

```sh
xcode-select --install
```

**Ubuntu / Debian / Raspberry Pi OS：**

```sh
sudo apt update
sudo apt install build-essential libncurses-dev libcrypt-dev
```

**Fedora：** `sudo dnf install gcc make ncurses-devel libxcrypt-devel`  
**Arch Linux：** `sudo pacman -S --needed base-devel ncurses libxcrypt`

Linuxでは、先に次のコマンドでビルド環境を設定します。macOSは通常、`make`のみでもビルドできます。

```sh
./configure
```

## 2. ビルド

```sh
make clean
make
```

成功すると実行ファイル **`rogue`** が作成されます。通常ビルドではOSの言語環境から起動時の言語を自動選択します。`make original`では英語を初期表示にします（ゲーム内容やその他の初期設定を変更するものではありません）。

| 目的 | コマンド |
|---|---|
| 通常ビルド | `make` |
| 英語を初期表示にする | `make original` |
| ウィザード機能を組み込む | `make wizard` |
| 実行ファイル名を `rogue36` にする | `make PROGRAM=rogue36`（一時指定）または `./configure --program=rogue36` → `make` |

実行ファイル名を変更した場合は、以下の`rogue`をその名前に置き換えてください。`./configure`の設定は`config.mk`に記録されます。ビルド設定を変更したら、必要に応じて`make clean`してから再ビルドしてください。

## 3. 起動

```sh
./rogue
```

通常起動では新しいゲームが始まります。ダンジョンの表示範囲は **80列×24行に固定** されているため、端末も80列×24行以上にしてください。

スコア一覧だけを表示するには、次のように実行します。

```sh
./rogue -s
```

詳しい操作方法は冒頭の案内書をご覧ください。

## 4. インストール（任意）

ビルドしたフォルダから直接起動する場合は、インストール不要です。通常は次のコマンドで`/usr/local`以下にインストールします。

```sh
sudo make install
```

インストール先を変更するには、事前に`./configure --prefix=DIR`を指定します。実行ファイル名は`--program=NAME`で指定できます。`make uninstall`でアンインストールできます。

## 5. セーブ・スコアなどの保存先

指定しなければ、セーブ **`rogue.save`**、スコア **`rogue.scr`**、ロック **`rogue.lock`** はゲーム起動時の作業フォルダに置かれます。実行ファイル名を変えると、これらの既定名も連動します。

**3種類のファイルを同じフォルダにまとめる例：**

```sh
mkdir -p "$HOME/.local/share/rogue36"
./configure --file-dir="$HOME/.local/share/rogue36"
make clean
make
```

保存先はゲームを実行するユーザーが書き込める必要があります。`--file-dir`で指定したフォルダは**`make install`の際だけ**必要に応じて作成されますが、`sudo`で作成すると書き込み権限に注意が必要なため、上の例では先に自分で作成しています。

**セーブ・スコア・ロックの保存先を分ける場合：**

```sh
mkdir -p "$HOME/Games/Rogue/saves" "$HOME/Games/Rogue/scores"
./configure --savedir="$HOME/Games/Rogue/saves" \
            --scoredir="$HOME/Games/Rogue/scores" \
            --lockdir="$HOME/Games/Rogue/scores"
make clean
make
```

ファイル名は`./configure --savefile=NAME`、`--scorefile=NAME`、`--lockfile=NAME`で個別に指定できます。また、`make PROGRAM=NAME FILEDIR=DIR`または従来の`make PROG=NAME FILEDIR=DIR`も利用できます。両方を指定した場合は`PROG=`を優先します。`./configure --program=NAME`は設定を`config.mk`に保存しますが、`make PROGRAM=NAME`はその実行時だけ名前を変更します。設定を変更したら再ビルドしてください。

## 6. ファイルの説明と配布

| ファイル | 用途 |
|---|---|
| `rogue` | ビルド後のゲーム本体 |
| `rogue.save` / `rogue.scr` / `rogue.lock` | セーブ／スコア／ロック |
| `configure` / `Makefile` / `Makefile.win` | ビルドとインストール |
| `dist-src.mk` | ソース配布アーカイブの収録ファイル一覧と生成処理 |
| `config.mk` / `config.log` | `configure`実行時に生成される設定・確認ログ |
| `*.c` / `*.h` | ルートディレクトリのゲーム本体ソースコード |
| `jec/` | JEC独自の共通機能（言語切り替え、Unicode文字列処理、画面表示補助など） |
| `windows/` | Windows専用のソースコードとアイコン関連ファイル |
| `original-notes/` | オリジナルRogue 3.6の歴史的な付属資料（通常ビルドでは使用しません） |
| `rogue_ja.html` / `rogue_en.html` | 「【運命の洞窟】への案内書」の日本語版／英語版（オフライン閲覧可） |
| `rogue_ja.txt` / `rogue_en.txt` | 同じ案内書の日本語版／英語版TXT（等幅フォント向け） |
| `README.md` | GitHub・ソース配布用の概要、OS・言語別READMEと案内書へのリンク |
| `LICENSE.txt` | ライセンス |

**ソース配布アーカイブの作成：** `make dist.src` を実行すると、ソースコード、日英README・案内書、ライセンス、Windows用ファイルを含む `rogue36_004-src.tar.gz` を作成します。ビルドや `make clean` は実行せず、セーブ・スコア・実行ファイル、設定ファイルなどは変更も収録もしません。Mac固有のメタデータも除外します。同名のアーカイブがすでにある場合は上書きせずに終了します。別名で作る場合は `make dist.src DIST_FILE=別名.tar.gz` を使用してください。

## 7. 著作権・ライセンス

オリジナルRogueおよび既存のコードの著作権・ライセンス、JEC版での変更や謝辞については、[LICENSE.txt](LICENSE.txt)をご覧ください。
