# Rogue 3.6 — Windows（日本語）

この文書では、**Windows 10 / 11** でのビルド、起動、環境変数、セーブ・スコアなどの保存先について説明します。Rogue 3.6とJEC版の紹介、および各OS・言語向け文書の一覧は[リポジトリのREADME](README.md)をご覧ください。操作方法や道具・オプションの説明は、同梱の[「【運命の洞窟】への案内書」](rogue_ja.html)にまとめています。

English version: [README_windows_en.md](README_windows_en.md)

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

標準ではセーブ **`rogue.save`**、スコア **`rogue.scr`** はゲーム起動時の作業フォルダに置かれます。実行ファイル名を変えると、既定のデータファイル名も連動します。

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

Windowsの「保存したゲーム」（Saved Games）の実際の保存先を取得し、その下の `Rogue/3.6` フォルダにセーブ・スコアファイルを保存します。5.4版のセーブファイルやスコアと混在しないよう、3.6専用のフォルダを使用します。保存先が変更・移動されていても取得でき、フォルダがなければビルド時に自動作成します。この機能には **MSYS2 UCRT64とPowerShell** が必要です。

**注意：この設定ではビルドしたユーザーの絶対パスがEXEに埋め込まれます。別のPCへ配布する実行ファイルでは `FILEDIR=saved-games` を使用せず、`FILEDIR` を指定しない通常ビルドにしてください。**

**保存先を分ける場合：**

```sh
mkdir -p rogue-data/saves rogue-data/scores
make clean
make SAVEDIR=rogue-data/saves \
     SCOREDIR=rogue-data/scores
```

ファイル名は `SAVEFILE=NAME`、`SCOREFILE=NAME` で変更できます。`SAVEDIR=DIR`、`SCOREDIR=DIR` を個別に指定した場合は、対応する保存先について `FILEDIR` より優先されます。保存先やファイル名を変更した場合は再ビルドしてください。相対パスはゲーム起動時の作業フォルダを基準にします。

## 6. 配布ファイルと説明書

| ファイル | 用途 |
|---|---|
| `rogue.exe` | ビルド後のゲーム本体 |
| `rogue.save` / `rogue.scr` | セーブ／スコア |
| `Makefile` / `Makefile.win` | ビルド用 |
| `dist-src.mk` | ソース配布アーカイブの収録ファイル一覧と生成処理 |
| `windows/rogue.ico` / `windows/rogue-icon.rc` | Windows用アイコン |
| `windows/windows-saved-games.ps1` | Windowsの「保存したゲーム」の場所の取得・3.6用フォルダ作成 |
| `windows/guide_windows_txt.sed` | Windows配布ZIP用にTXT案内書のコマンド例を調整するルール |
| `*.c` / `*.h` | ルートディレクトリのゲーム本体ソースコード |
| `jec/` | JEC独自の共通機能（言語切り替え、Unicode文字列処理、画面表示補助など） |
| `windows/` | Windows専用のソースコードとアイコン関連ファイル |
| `original-notes/` | オリジナルRogue 3.6の歴史的な付属資料（通常ビルドでは使用しません） |
| `rogue_ja.html` / `rogue_en.html` | 「【運命の洞窟】への案内書」の日本語版／英語版（オフライン閲覧可） |
| `rogue_ja.txt` / `rogue_en.txt` | 同じ案内書の日本語版／英語版TXT（等幅フォント向け） |
| `README.md` | GitHub・ソース配布用の概要、OS・言語別READMEと案内書へのリンク |
| `LICENSE.txt` | ライセンス |

ソースコードをビルドしないで遊ぶだけなら、MSYS2の開発環境は不要です。

**ソース配布アーカイブの作成：** `make dist.src` を実行すると、ソースコード、日英README・案内書、ライセンス、Windows用ファイルを含む `rogue36_007-src.tar.gz` を作成します。ビルドや `make clean` は実行せず、セーブ・スコア・実行ファイル、設定ファイルなどは変更も収録もしません。Mac固有のメタデータも除外します。同名のアーカイブがすでにある場合は上書きせずに終了します。別名で作る場合は `make dist.src DIST_FILE=別名.tar.gz` を使用してください。

**Windowsバイナリ配布ZIPの作成：** MSYS2 UCRT64で、別途`zip`コマンドをインストールしてから実行します。

```sh
pacman -S --needed zip
make dist.win
```

`rogue36_007_windows.zip`が作成されます。通常の`make`は`rogue.exe`を作成しますが、`make dist.win`は配布用に`rogue36.exe`をビルドし、`windows/binary/`に収録された日英READMEと日英HTML・TXT案内書、`LICENSE.txt`とともに`Rogue 3.6/`フォルダーへまとめます。配布文書はGitHub Releasesの`v004`にあるWindows版ZIPを基準にしています。TXT案内書のコマンド例はWindows用に調整し、ソース版の案内書とREADMEは書き換えません。

実行ファイル名を変更する場合は`make PROGRAM=別名 dist.win`を指定できます。配布用EXEはセーブ・スコアを起動時の作業フォルダーに保存します。`FILEDIR=saved-games`など特定のPCに固定された保存先は指定できません。同名ZIPが存在する場合は上書きせず終了します。

## 7. 著作権・ライセンス

オリジナルRogueおよび既存のコードの著作権・ライセンス、JEC版での変更や謝辞については、[LICENSE.txt](LICENSE.txt)をご覧ください。
