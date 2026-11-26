# Adapt the preformatted TXT guides only when packaging for Windows.
# Preserve the source guides and the original line/page count.
# Replacement lines are kept within the original text measure.
s|^               ぐためである。再開するには「rogue セーブファイル名」と入力$|               ぐためである。Windows版で再開するには|
s|^   setenv ROGUEOPTS "jump,noterse,name=ブルー・ミーニー"\[3\]$|   set "ROGUEOPTS=jump,noterse,name=ブルー・ミーニー"[3]|
s|^ file\[プログラム名\.save\]（セーブファイル名）$| file[rogue36.save]（セーブファイル名）|
s|^        ゲームをセーブするためのデフォルトのファイル名。通常は「rogue\.sav$|        ゲームをセーブするためのデフォルトのファイル名。Windowsバイナリ|
s|^        e」であり、実行ファイル名を変更した場合はそれに応じて変わる。電話$|        配布版では「rogue36.save」である。実行ファイルの名前を後で変えても|
s|^        回線がアクシデントでハングアップした場合、ローグは自動的にこのファ$|        既定のファイル名は変わらない。電話回線がアクシデントでハングアップ|
s|^        イルにゲームをセーブする。ファイル名はホームディレクトリとして展開$|        した場合、ローグはこのファイルにゲームをセーブする。ファイル名は|
s|^        される特殊文字である「~」で始めても良い。$|        ホームディレクトリとして展開される特殊文字「~」で始めても良い。|
s|^ 3 Bourneシェルを使用する場合は、次のように設定する。$| 3 PowerShellを使用する場合は、次のように設定して起動する。|
s|^   ROGUEOPTS="jump,noterse,name=ブルー・ミーニー"$|   $env:ROGUEOPTS="jump,noterse,name=ブルー・ミーニー"|
s|^   export ROGUEOPTS$|   .\\rogue36.exe|
s|^             filename as “rogue save_file”\.$|             filename as “.\\rogue36.exe save_file”.|
s|^ % setenv ROGUEOPTS "jump,noterse,name=Conan the Barbarian,fruit=mango"\[3\]$|> set "ROGUEOPTS=jump,noterse,name=Conan the Barbarian,fruit=mango"[3]|
s|^  3 For those of you who use the Bourne shell, the commands would be$|  3 For PowerShell users, the corresponding commands are|
s|^    \$ ROGUEOPTS="jump,noterse,name=Conan the Barbarian,fruit=mango"$|    PS> $env:ROGUEOPTS="jump,noterse,name=Conan the Barbarian,fruit=mango"|
s|^    \$ export ROGUEOPTS$|    PS> .\\rogue36.exe|
s|^ file \[PROGRAM\.save\]$| file [rogue36.save]|
s|^        The default file name for saving the game\. The usual default is$|        The default file name for saving the game. In the Windows binary|
s|^        rogue\.save; the name changes with the executable name\. If your$|        distribution it is rogue36.save; renaming the EXE does not change|
s|^        phone is hung up by accident, rogue will automatically save the$|        that setting. If your phone is hung up by accident, rogue will|
s|^        game in this file\. The file name may begin with the special$|        automatically save the game in this file. The file name may begin|
s|^        character “~”, which expands to your home directory\.$|        with “~”, which expands to your home directory.|
/^               ぐためである。Windows版で再開するには$/ {
  n
  s|^               する。$|               「.\\rogue36.exe セーブファイル名」と入力する。|
}
