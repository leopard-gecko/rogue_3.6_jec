#!/bin/sh
# Validate the distributable Windows ZIP produced by "make dist.win".
# Run in the repository root under MSYS2 UCRT64:
#   sh windows/check-dist-win.sh rogue36_007_windows.zip
# Never modifies the source guides or uses existing player data files.
set -eu

if [ "$#" -ne 1 ]; then
    echo 'Usage: sh windows/check-dist-win.sh <Windows distribution ZIP>' >&2
    exit 2
fi
archive=$1
if [ ! -s "$archive" ]; then
    echo "Windows distribution ZIP is missing or empty: $archive" >&2
    exit 1
fi

for tool in unzip objdump sed grep cmp sort tr mktemp; do
    if ! command -v "$tool" >/dev/null 2>&1; then
        echo "Required test tool is missing: $tool" >&2
        exit 1
    fi
done

# The archive must contain one root folder and exactly eight regular files.
# Checking an exact manifest also rejects unwanted build outputs or paths.
unzip -tq "$archive"
stage=$(mktemp -d "${TMPDIR:-/tmp}/rogue36-dist-check.XXXXXXXX")
trap 'rm -rf "$stage"' 0
trap 'exit 1' 1 2 3 15
unzip -Z1 "$archive" | LC_ALL=C sort > "$stage/members.actual"
cat <<'MEMBERS' | LC_ALL=C sort > "$stage/members.expected"
Rogue 3.6/
Rogue 3.6/rogue36.exe
Rogue 3.6/README_ja.txt
Rogue 3.6/README_en.txt
Rogue 3.6/rogue_ja.html
Rogue 3.6/rogue_en.html
Rogue 3.6/rogue_ja.txt
Rogue 3.6/rogue_en.txt
Rogue 3.6/LICENSE.txt
MEMBERS
if ! cmp -s "$stage/members.expected" "$stage/members.actual"; then
    echo 'Unexpected Windows ZIP members (expected one folder and eight files):' >&2
    echo 'Expected:' >&2
    cat "$stage/members.expected" >&2
    echo 'Actual:' >&2
    cat "$stage/members.actual" >&2
    exit 1
fi
unzip -q "$archive" -d "$stage"
package="$stage/Rogue 3.6"
exe="$package/rogue36.exe"
test -s "$exe"

# Verify that bundled documents are the intended release copies and that
# TXT guides, unlike source guides, were adapted for Windows at packaging.
for doc in README_ja.txt README_en.txt rogue_ja.html rogue_en.html; do
    cmp "windows/binary/$doc" "$package/$doc"
done
cmp LICENSE.txt "$package/LICENSE.txt"
for guide in rogue_ja.txt rogue_en.txt; do
    sed -f windows/guide_windows_txt.sed "$guide" | cmp - "$package/$guide"
    if cmp -s "$guide" "$package/$guide"; then
        echo "Windows TXT guide was not adapted: $guide" >&2
        exit 1
    fi
    grep -Fq 'rogue36.exe' "$package/$guide"
    grep -Fq 'rogue36.save' "$package/$guide"
done

# The packaged program must be an x86-64 Windows PE binary and must not
# require PDCurses or MSYS2 DLLs on the end user's PC.  Fail closed on
# unknown DLL imports rather than merely printing the imports in a log.
objdump -f "$exe" | grep -Fq 'pei-x86-64'
objdump -p "$exe" > "$stage/imports.txt"
sed -n 's/^[[:space:]]*DLL Name:[[:space:]]*//p' \
    "$stage/imports.txt" > "$stage/dlls.txt"
if [ ! -s "$stage/dlls.txt" ]; then
    echo 'No PE DLL imports found; dependency verification failed' >&2
    exit 1
fi
while IFS= read -r dll; do
    lower=$(printf '%s' "$dll" | tr '[:upper:]' '[:lower:]')
    case "$lower" in
        advapi32.dll|bcrypt.dll|comctl32.dll|comdlg32.dll|crypt32.dll|dwmapi.dll|gdi32.dll|imm32.dll|kernel32.dll|kernelbase.dll|msvcrt.dll|ntdll.dll|ole32.dll|oleaut32.dll|psapi.dll|rpcrt4.dll|secur32.dll|shell32.dll|shlwapi.dll|ucrtbase.dll|user32.dll|version.dll|winmm.dll|ws2_32.dll|api-ms-win-*.dll|ext-ms-*.dll)
            ;;
        *)
            echo "Non-system DLL dependency in distributed EXE: $dll" >&2
            exit 1
            ;;
    esac
done < "$stage/dlls.txt"

# The -s command is noninteractive even when output is redirected.  Execute
# the ZIP's own EXE from a fresh working directory to verify both language
# overrides and the bundled program's default score-file name.
mkdir "$stage/smoke"
(
    cd "$stage/smoke"
    env ROGUEOPTS=language=en "$exe" -s > score-en.txt
    grep -Fq 'Adventurers:' score-en.txt
    test -s rogue36.scr
    test ! -e rogue.scr
    test ! -e rogue36.save
    env ROGUEOPTS=language=ja "$exe" -s > score-ja.txt
    grep -Fq 'トップ' score-ja.txt
    grep -Fq '冒険者' score-ja.txt
    test -s rogue36.scr
    test ! -e rogue.scr
    test ! -e rogue36.save
)

echo 'Windows ZIP verified: 8 files, docs, x86-64 PE, system DLLs only, both languages and rogue36.scr.'
