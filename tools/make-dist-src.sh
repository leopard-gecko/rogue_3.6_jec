#!/bin/sh
# Create a source-only tarball without modifying or cleaning the source tree.
# Uses a fixed manifest to exclude objects, binaries, saves, scores, logs,
# operating-system metadata and other untracked local files.
set -eu

name=${1:-rogue36_004}
archive=${2:-$name-src.tar.gz}

case "$name" in
    ''|.|..|-*|*[!a-zA-Z0-9._-]*)
        echo "dist.src: invalid archive directory name: $name" >&2
        exit 1 ;;
esac
case "$archive" in
    ''|*/) echo 'dist.src: invalid archive filename' >&2; exit 1 ;;
esac

# Resolve paths before changing directory; no build outputs are touched.
source_dir=$(CDPATH= cd "$(dirname "$0")/.." && pwd -P)
manifest="$source_dir/tools/dist-src-files.txt"
out_dir=$(dirname "$archive")
out_base=$(basename "$archive")
out_dir=$(CDPATH= cd "$out_dir" && pwd -P) || {
    echo "dist.src: output directory not found: $out_dir" >&2
    exit 1
}
output="$out_dir/$out_base"
if [ -e "$output" ] || [ -L "$output" ]; then
    echo "dist.src: output already exists (not overwritten): $output" >&2
    exit 1
fi

# Disable macOS copyfile/AppleDouble sidecar metadata in cp and tar.
COPYFILE_DISABLE=1
export COPYFILE_DISABLE

# mktemp owns the only directory that the cleanup trap may remove.
tmp_dir=$(mktemp -d "${TMPDIR:-/tmp}/rogue36-dist.XXXXXXXX") || exit 1
tmp_archive=''
cleanup() {
    if [ -n "$tmp_archive" ]; then rm -f "$tmp_archive"; fi
    if [ -n "$tmp_dir" ]; then rm -rf "$tmp_dir"; fi
}
trap cleanup 0
trap 'exit 1' 1 2 3 15
stage="$tmp_dir/$name"
mkdir "$stage"

# Reject suspicious manifest paths rather than accidentally shipping local data.
while IFS= read -r entry || [ -n "$entry" ]; do
    case "$entry" in ''|\#*) continue ;; esac
    case "$entry" in
        /*|..|../*|*/..|*/../*|./*|*/./*|*\\*)
            echo "dist.src: invalid manifest path: $entry" >&2; exit 1 ;;
    esac
    if [ ! -f "$source_dir/$entry" ] || [ -L "$source_dir/$entry" ]; then
        echo "dist.src: missing or symlinked source file: $entry" >&2
        exit 1
    fi
    entry_dir=$(dirname "$entry")
    if [ "$entry_dir" != '.' ]; then mkdir -p "$stage/$entry_dir"; fi
    cp "$source_dir/$entry" "$stage/$entry"
done < "$manifest"

# The configure command and packaging script must remain runnable after unpacking.
chmod +x "$stage/configure" "$stage/tools/make-dist-src.sh"

# Build fully into a temporary file in the *destination filesystem*, so a
# failed tar run never leaves a partially written release archive.
tmp_archive=$(mktemp "$out_dir/.rogue36-src.XXXXXXXX") || exit 1
( cd "$tmp_dir" && tar -czf "$tmp_archive" "$name" )

# No implicit overwrite of an earlier release, even on repeated invocations.
if [ -e "$output" ] || [ -L "$output" ]; then
    echo "dist.src: output already exists (not overwritten): $output" >&2
    exit 1
fi
mv "$tmp_archive" "$output"
tmp_archive=''
echo "dist.src: created $output"
