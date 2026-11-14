# Rogue 3.6 — Windows（日本語）

Rogueは、1980年頃に誕生した、文字や記号で描かれる迷宮を探索するコンピューターRPGです。プレイヤーは怪物と戦い、さまざまな道具を駆使しながら、迷宮の奥に隠された「イェンダーの魔除け」を探します。

**Rogue 3.6は、1981年に広く普及した、現在入手できる最古のRogueです。** Rogue誕生当時の姿を今に伝える歴史的にも貴重なバージョンであり、後のRogue 5.2や5.4とは異なる怪物や道具、ゲームの仕組みなど、初期のRogueならではの特徴が残されています。

この修正版では、オリジナルのゲーム内容をできるだけ尊重しながら、日本語・英語の切り替えと現代のOSでの動作に対応しています。

ゲームの概要・操作方法・遊び方、アイテムなどについては、同梱の[日本語版「【運命の洞窟】への案内書」](rogue_ja.html)、または[英語版「A Guide to the Dungeons of Doom」](rogue_en.html)をご覧ください。どちらもWebブラウザーで開くことができ、インターネット接続は不要です。案内書は[Web版](https://leopard-gecko.github.io/jrogue/guide.html?version=36)でも公開しています。このREADMEでは、開発環境の準備、ビルド、起動、Windowsでの環境変数の設定、保存先、配布ファイルなどを説明します。

この説明書は **Windows 10 / 11向け** です。ほかの説明書：[Windows・英語](README_windows_en.md) ／ [macOS / Linux・日本語](README_mac_linux_ja.md) ／ [macOS / Linux・英語](README_mac_linux_en.md)

## 1. 開発環境の準備

Windows版をビルドするには **MSYS2 UCRT64** を使用します。<https://www.msys2.org/> からMSYS2をインストールし、スタートメニューの「MSYS2 UCRT64」を開いてください。

```sh
pacman -Syu
```

更新時に再起動を求められたらUCRT64を開き直し、更新を完了させてください。続いて必要なパッケージをインストールします。

```sh
pacman -S --needed make mingw-w64-ucrt-x86_64-gcc mingw-w64-ucrt-x86_64-binutils mingw-w64-ucrt-x86_64-pdcurses
```

ソースコードを展開し、UCRT64でそのフォルダへ移動してください。

## 2. ビルド

```sh
make clean
make
```

成功すると **`rogue.exe`** が作成されます。Windows版はVT方式の描画を使用します。Windowsでのビルドに`./configure`や`make install`は不要です。

| 目的 | コマンド |
|---|---|
| 通常ビルド | `make` |
| 英語を初期表示にした`rogue_original.exe`を作る | `make PROGRAM=rogue_original original` |
| ウィザード機能を組み込む | `make wizard` |
| 実行ファイル名を`rogue36.exe`にする | `make PROGRAM=rogue36` |

`PROG=`も従来どおり使用できます。`PROGRAM=`と`PROG=`を同時に指定した場合は`PROG=`を優先します。`make original`は英語を初期表示にしますが、ゲーム内容やその他の初期設定は変更しません。ビルド設定を変更するときは、必要に応じて`make clean`してから作り直してください。

## 3. 起動と終了

エクスプローラーから **`rogue.exe`をダブルクリック**すると、**標準のセーブファイルがない場合はメニューなしで新しい冒険が始まります**。標準のセーブファイルが見つかった場合にだけ、次の起動メニューが表示されます。

| キー | 動作 |
|---|---|
| `R` / `Enter` | 標準のセーブファイルから冒険を再開する |
| `N` | 新しい冒険を始める |
| `F` | Windowsのファイル選択画面で別のセーブファイルを選ぶ |
| `Q` | 起動せずに終了する |

標準のセーブファイルがないときでも、**セーブファイルを`rogue.exe`のアイコンにドラッグ＆ドロップ**すれば、そのファイルから再開できます。日本語を含むフォルダ名やファイル名にも対応しています。起動メニューでファイル選択をキャンセルするとメニューに戻ります。新規開始を選んでも既存のセーブファイルは削除されません。再開に失敗したときはエラーを表示し、Enterを押すまでウィンドウを保持します。

PowerShellやコマンドプロンプト、MSYS2 UCRT64から起動した場合は、従来どおり起動メニューを表示せず、新しいゲームを開始します。

```powershell
.\rogue.exe
```

コマンドからセーブファイルを指定して再開する場合は、`.\rogue.exe`にファイルのパスを引数として渡してください（例：`.\rogue.exe rogue.save`）。

ダンジョンは **80列×24行に固定** されています。ダブルクリック起動では、Windows TerminalやWindows Console Hostのウィンドウも80×24への調整を試みます。ゲーム終了時にはゲーム画面を消去してスコア一覧を表示し、Enterを押すまでウィンドウを保持します。既存の端末からコマンドで起動した場合は、端末のサイズ変更と終了時の入力待ちは行いません。

スコア一覧だけを表示するには、PowerShellから次のように実行します。

```powershell
.\rogue.exe -s
```

詳しい操作方法は冒頭の案内書をご覧ください。

## 4. Windowsでの環境変数の設定

起動時の設定には環境変数 **`ROGUEOPTS`** を使用できます。ここではWindowsでの設定方法を説明します。設定項目の意味は[日本語版案内書](rogue_ja.html)の「オプション」をご覧ください。

**開いている端末内で一時的に設定する場合：** 以下の`terse`は設定文字列の例です。

PowerShell：

```powershell
$env:ROGUEOPTS = 'terse'
.\rogue.exe
```

コマンドプロンプト：

```cmd
set "ROGUEOPTS=terse"
rogue.exe
```

MSYS2 UCRT64：

```sh
export ROGUEOPTS='terse'
./rogue.exe
```

これらは開いている端末と、そこから起動するプログラムに適用されます。端末を閉じると設定は失われます。

**ダブルクリック起動でも設定を使う場合：**

1. `Win + R`を押し、`sysdm.cpl`を実行します。
2. 「詳細設定」→「環境変数」を開きます。
3. 「ユーザー環境変数」で「新規」または「編集」を選び、変数名を`ROGUEOPTS`、変数値を設定文字列にします。
4. 「OK」で保存します。反映されない場合はWindowsにサインインし直してください。

設定が不要になったら、同じ画面で`ROGUEOPTS`を削除できます。`FILEDIR`などの保存先は、環境変数ではなく次のビルド時設定です。

## 5. セーブ・スコアなどの保存先

標準ではセーブ **`rogue.save`**、スコア **`rogue.scr`**、ロック **`rogue.lock`** はゲーム起動時の作業フォルダに置かれます。実行ファイル名を変えると、既定のデータファイル名も連動します。

**3種類のファイルを同じフォルダにまとめる例：**

```sh
mkdir -p rogue-data
make clean
make FILEDIR=rogue-data
```

通常の `FILEDIR` 指定ではフォルダは自動作成されません。**先に作成し、書き込み権限を確保してください。** ほかの場所に保存する場合の例：

```sh
mkdir -p '/c/Users/Public/Rogue36Data'
make clean
make FILEDIR='C:/Users/Public/Rogue36Data'
```

フォルダの作成だけを行う場合は `make prepare-filedir FILEDIR='C:/Users/Public/Rogue36Data'` も利用できます。

**Windowsの「保存したゲーム」を利用する場合（個人用ビルド）：**

```sh
make clean
make FILEDIR=saved-games
```

Windowsの「保存したゲーム」（Saved Games）の実際の保存先を取得し、その下の `Rogue/3.6` フォルダにセーブ・スコア・ロックファイルを保存します。5.4版のセーブファイルやスコアと混在しないよう、3.6専用のフォルダを使用します。保存先が変更・移動されていても取得でき、フォルダがなければビルド時に自動作成します。この機能には **MSYS2 UCRT64とPowerShell** が必要です。

**注意：この設定ではビルドしたユーザーの絶対パスがEXEに埋め込まれます。別のPCへ配布する実行ファイルでは `FILEDIR=saved-games` を使用せず、`FILEDIR` を指定しない通常ビルドにしてください。**

**保存先を分ける場合：**

```sh
mkdir -p rogue-data/saves rogue-data/scores
make clean
make SAVEDIR=rogue-data/saves \
     SCOREDIR=rogue-data/scores \
     LOCKDIR=rogue-data/scores
```

ファイル名は `SAVEFILE=NAME`、`SCOREFILE=NAME`、`LOCKFILE=NAME` で変更できます。`SAVEDIR=DIR`、`SCOREDIR=DIR`、`LOCKDIR=DIR` を個別に指定した場合は、対応する保存先について `FILEDIR` より優先されます。保存先やファイル名を変更した場合は再ビルドしてください。相対パスはゲーム起動時の作業フォルダを基準にします。

## 6. 配布ファイルと説明書

| ファイル | 用途 |
|---|---|
| `rogue.exe` | ビルド後のゲーム本体 |
| `rogue.save` / `rogue.scr` / `rogue.lock` | セーブ／スコア／ロック |
| `Makefile` / `Makefile.win` / `dist-src.mk` | ビルド用 |
| `windows/rogue.ico` / `windows/rogue-icon.rc` | Windows用アイコン |
| `windows/windows-saved-games.ps1` | Windowsの「保存したゲーム」の場所の取得・3.6用フォルダ作成 |
| `*.c` / `*.h` | ルートディレクトリのゲーム本体ソースコード |
| `jec/` | JEC独自の共通機能（言語切り替え、Unicode文字列処理、画面表示補助など） |
| `windows/` | Windows専用のソースコードとアイコン関連ファイル |
| `original-notes/` | オリジナルRogue 3.6の歴史的な付属資料（通常ビルドでは使用しません） |
| `rogue_ja.html` / `rogue_en.html` | 「【運命の洞窟】への案内書」の日本語版／英語版（オフライン閲覧可） |
| `tools/` | ソース配布アーカイブ作成用スクリプトと収録ファイル一覧 |
| `LICENSE.txt` | ライセンス |
| `README_windows_ja.md` / `README_windows_en.md` | Windows向け日英README |
| `README_mac_linux_ja.md` / `README_mac_linux_en.md` | macOS / Linux向け日英README |

作成した`rogue.exe`をほかのPCへコピーする場合は、必要な実行時DLLも一緒に用意してください。ソースコードをビルドしないで遊ぶだけなら、MSYS2の開発環境は不要です。

**ソース配布アーカイブの作成：** `make dist.src` を実行すると、ソースコード、日英README・案内書、ライセンス、Windows用ファイルを含む `rogue36_004-src.tar.gz` を作成します。ビルドや `make clean` は実行せず、セーブ・スコア・実行ファイル、設定ファイルなどは変更も収録もしません。Mac固有のメタデータも除外します。同名のアーカイブがすでにある場合は上書きせずに終了します。別名で作る場合は `make dist.src DIST_FILE=別名.tar.gz` を使用してください。

## 7. オリジナルのRogue 3.6からの主な変更点

- **Windows対応：** MSYS2 UCRT64でWindowsネイティブ版をビルドでき、アイコン付きの実行ファイルを作成できます。
- **日本語・英語：** メッセージ、アイテム、オプション画面、スコア表示を日本語化し、起動時やゲーム中に言語を切り替えられます。
- **日本語入力と表示：** 名前や設定のUTF-8文字・絵文字に対応し、メッセージ表示を改善しました。
- **端末表示：** ダンジョンを80列×24行に固定し、ダブルクリック起動時には端末のウィンドウサイズ調整を試みます。
- **スコア表示：** ダブルクリック起動ではゲーム画面を消してスコアを表示し、Enterを押すまで保持します。
- **保存と安定性：** 保存先の指定やスコア・ロック処理、ウィザードモードのパスワード入力などを改善しました。
