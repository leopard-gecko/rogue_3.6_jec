# Rogue 3.6 — macOS / Linux（日本語）

Rogueは、1980年頃に誕生した、文字や記号で描かれる迷宮を探索するコンピューターRPGです。プレイヤーは怪物と戦い、さまざまな道具を駆使しながら、迷宮の奥に隠された「イェンダーの魔除け」を探します。

**Rogue 3.6は、1981年に広く普及した、現在入手できる最古のRogueです。** Rogue誕生当時の姿を今に伝える歴史的にも貴重なバージョンであり、後のRogue 5.2や5.4とは異なる怪物や道具、ゲームの仕組みなど、初期のRogueならではの特徴が残されています。

この修正版では、オリジナルのゲーム内容をできるだけ尊重しながら、日本語・英語の切り替えと現代のOSでの動作に対応しています。

ゲームの概要・操作方法・遊び方、アイテムなどについては、同梱の[日本語版「【運命の洞窟】への案内書」](rogue_ja.html)、または[英語版「A Guide to the Dungeons of Doom」](rogue_en.html)をご覧ください。どちらもWebブラウザーで開くことができ、インターネット接続は不要です。案内書は[Web版](https://leopard-gecko.github.io/jrogue/guide.html?version=36)でも公開しています。このREADMEでは、開発環境の準備、ビルド、起動、保存先、配布ファイルなどを説明します。

この説明書は **macOS / Linux / Raspberry Pi OS向け** です。ほかの説明書：[macOS / Linux・英語](README_mac_linux_en.md) ／ [Windows・日本語](README_windows_ja.md) ／ [Windows・英語](README_windows_en.md)

## 1. 開発環境の準備

ソースコードを展開し、ターミナルでそのフォルダを開いてください。

**macOS：** Appleのコマンドラインツールをインストールします。

```sh
xcode-select --install
```

**Ubuntu / Debian / Raspberry Pi OS：**

```sh
sudo apt update
sudo apt install build-essential libncurses-dev
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
| `configure` / `Makefile` / `Makefile.win` / `dist-src.mk` | ビルドとインストール |
| `config.mk` / `config.log` | `configure`実行時に生成される設定・確認ログ |
| `*.c` / `*.h` | ルートディレクトリのゲーム本体ソースコード |
| `jec/` | JEC独自の共通機能（言語切り替え、Unicode文字列処理、画面表示補助など） |
| `windows/` | Windows専用のソースコードとアイコン関連ファイル |
| `original-notes/` | オリジナルRogue 3.6の歴史的な付属資料（通常ビルドでは使用しません） |
| `rogue_ja.html` / `rogue_en.html` | 「【運命の洞窟】への案内書」の日本語版／英語版（オフライン閲覧可） |
| `tools/` | ソース配布アーカイブ作成用スクリプトと収録ファイル一覧 |
| `LICENSE.txt` | ライセンス |
| `README_mac_linux_ja.md` / `README_mac_linux_en.md` | macOS / Linux向けの日英README |
| `README_windows_ja.md` / `README_windows_en.md` | Windows向けの日英README |

**ソース配布アーカイブの作成：** `make dist.src` を実行すると、ソースコード、日英README・案内書、ライセンス、Windows用ファイルを含む `rogue36_004-src.tar.gz` を作成します。ビルドや `make clean` は実行せず、セーブ・スコア・実行ファイル、設定ファイルなどは変更も収録もしません。Mac固有のメタデータも除外します。同名のアーカイブがすでにある場合は上書きせずに終了します。別名で作る場合は `make dist.src DIST_FILE=別名.tar.gz` を使用してください。

## 7. オリジナルのRogue 3.6からの主な変更点

- **現代のOSへの対応：** macOS、Linux、Raspberry Pi OSでビルドして遊べるようにしました。
- **日本語・英語：** メッセージ、アイテム名、オプション画面、スコア表示などを日本語化し、起動時やゲーム中に言語を切り替えられるようにしました。
- **表示：** UTF-8の日本語表示、メッセージ再表示、長い文字列の扱いを改善し、ダンジョン画面を80列×24行に固定しました。
- **保存とビルド：** 実行ファイル名、インストール先、セーブ・スコア・ロックの保存先を指定できるようにしました。
- **安定性：** 戦闘、アイテム、画面表示、ウィザードモードのパスワード入力などの不具合を修正しました。
