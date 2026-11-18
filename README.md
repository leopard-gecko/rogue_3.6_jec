# Rogue 3.6 JEC

Rogueは、1980年頃に誕生した、文字や記号で描かれる迷宮を探索するコンピューターRPGです。プレイヤーは怪物と戦い、さまざまな道具を使いながら、迷宮の奥に隠された「イェンダーの魔除け」を探します。いわゆる「ローグライクゲーム」の元祖です。

**Rogue 3.6** は、1981年に広く普及した、現在入手できる最古のRogueのバージョンです。後のRogue 5.2や5.4とは異なる怪物、道具、ゲームの仕組みなど、初期のRogueならではの特徴が残されています。

**Rogue 3.6 JEC** は、元のRogue 3.6のゲーム内容はそのままに、日本語・英語の切り替え、日本語や絵文字の入出力、現代のOSへの対応、表示やセーブ処理の改善などを加えた修正版です。

*Rogue 3.6 JEC is a modified edition of one of the earliest surviving versions of Rogue. It preserves the original gameplay as closely as possible while adding Japanese/English language switching, UTF-8 text support, improvements to display and save handling, and compatibility with modern operating systems.*

## 対応環境とREADME / Platforms and documentation

macOS、Linux（Raspberry Pi OSを含む）、Windows 10 / 11に対応しています。ビルド方法、起動方法、オプションやセーブファイルの扱いについては、OS・言語別のREADMEをご覧ください。

*For build instructions, launching, options, and save-file locations, choose the README for your operating system and language.*

| 対象OS / Platform | 日本語 | English |
| --- | --- | --- |
| macOS / Linux / Raspberry Pi OS | [README（日本語）](README_mac_linux_ja.md) | [README (English)](README_mac_linux_en.md) |
| Windows 10 / 11 | [README（日本語）](README_windows_ja.md) | [README (English)](README_windows_en.md) |

macOS / Linuxでは通常 `make` を使用し、WindowsではMSYS2 UCRT64環境でビルドできます。必要な準備や詳しい手順は各READMEを参照してください。

*On macOS/Linux, the standard build uses `make`. Windows builds use MSYS2 UCRT64; see the platform-specific README for prerequisites and details.*

## 【運命の洞窟】への案内書 / A Guide to the Dungeons of Doom

ゲームの操作、画面表示、道具、オプションなどは、日英の案内書にまとめています。HTMLファイルはWebブラウザーで、TXTファイルは等幅フォントのテキスト表示で閲覧できます。どちらもインターネット接続は不要です。

*The Japanese and English dungeon guides describe the game, commands, items, and options. The HTML guides can be viewed in a web browser, while the TXT versions are formatted for monospaced text. Both formats are available offline.*

| 言語 / Language | HTML | TXT |
| --- | --- | --- |
| 日本語 | [rogue_ja.html](rogue_ja.html) | [rogue_ja.txt](rogue_ja.txt) |
| English | [rogue_en.html](rogue_en.html) | [rogue_en.txt](rogue_en.txt) |

[公開サイトの案内書](https://leopard-gecko.github.io/jrogue/guide.html?version=36)も利用できます。

*The [online dungeon guide](https://leopard-gecko.github.io/jrogue/guide.html?version=36) is also available.*

## 著作権・ライセンス / Copyright and license

オリジナルRogueおよび既存のコードの著作権・ライセンス、JEC版での変更と謝辞については、[LICENSE.txt](LICENSE.txt) をご覧ください。

*See [LICENSE.txt](LICENSE.txt) for copyright notices, license terms, JEC modifications, and acknowledgments.*
