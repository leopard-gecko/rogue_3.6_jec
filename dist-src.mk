# Shared source-only archive target for Unix and MSYS2 Windows builds.
# Everything needed for packaging is kept here: the file list and the recipe.
# Packaging never compiles, cleans, or changes the working source tree.
DIST_NAME ?= rogue36_004
DIST_FILE ?= $(DIST_NAME)-src.tar.gz

# Explicit allow-list: no objects, executables, saved games, configuration,
# untracked files, or macOS metadata are copied into the source release.
DIST_SRC_FILES := \
    .github/workflows/build.yml \
    LICENSE.txt \
    Makefile \
    Makefile.win \
    README.md \
    README_mac_linux_en.md \
    README_mac_linux_ja.md \
    README_windows_en.md \
    README_windows_ja.md \
    armor.c \
    chase.c \
    command.c \
    configure \
    daemon.c \
    daemons.c \
    dist-src.mk \
    fight.c \
    getpass.c \
    globals.c \
    init.c \
    io.c \
    jec/language.c \
    jec/language.h \
    jec/screen_text.c \
    jec/screen_text.h \
    jec/unicode_output.h \
    jec/unicode_text.c \
    jec/unicode_text.h \
    list.c \
    mach_dep.h \
    main.c \
    misc.c \
    monsters.c \
    move.c \
    new_level.c \
    options.c \
    original-notes/README.md \
    original-notes/edd \
    original-notes/findpw.c \
    original-notes/initsfile.c \
    original-notes/listfile \
    original-notes/mes \
    pack.c \
    passages.c \
    potions.c \
    rings.c \
    rip.c \
    rogue.h \
    rogue_en.html \
    rogue_en.txt \
    rogue_ja.html \
    rogue_ja.txt \
    rooms.c \
    save.c \
    scrolls.c \
    sticks.c \
    things.c \
    vers.c \
    weapons.c \
    windows/rogue-icon.rc \
    windows/rogue.ico \
    windows/rogue_windows.c \
    windows/rogue_windows.h \
    windows/rogue_windows_console.c \
    windows/rogue_windows_console.h \
    windows/windows-saved-games.ps1 \
    windows/unicode_output.c \
    wizard.c \
    xcrypt.c

# Quote Make variables as single POSIX-shell arguments (including spaces).
dist_shell_quote = '$(subst ','"'"',$(1))'

# Link the finished archive into place exclusively instead of overwriting.
.PHONY: dist.src
dist.src:
	@set -eu; \
	name=$(call dist_shell_quote,$(DIST_NAME)); \
	archive=$(call dist_shell_quote,$(DIST_FILE)); \
	case "$$name" in \
		''|.|..|-*|*[!a-zA-Z0-9._-]*) \
			echo "dist.src: invalid archive directory name: $$name" >&2; exit 1 ;; \
	esac; \
	case "$$archive" in \
		''|*/) echo 'dist.src: invalid archive filename' >&2; exit 1 ;; \
	esac; \
	source_dir=$$(pwd -P); \
	out_dir=$$(dirname "$$archive"); \
	out_base=$$(basename "$$archive"); \
	out_dir=$$(CDPATH= cd "$$out_dir" && pwd -P) || { \
		echo "dist.src: output directory not found: $$out_dir" >&2; exit 1; \
	}; \
	output="$$out_dir/$$out_base"; \
	if [ -e "$$output" ] || [ -L "$$output" ]; then \
		echo "dist.src: output already exists (not overwritten): $$output" >&2; exit 1; \
	fi; \
	COPYFILE_DISABLE=1; export COPYFILE_DISABLE; \
	tmp_dir=$$(mktemp -d "$${TMPDIR:-/tmp}/rogue36-dist.XXXXXXXX"); \
	tmp_archive=''; \
	cleanup() { \
		if [ -n "$$tmp_archive" ]; then rm -f "$$tmp_archive"; fi; \
		if [ -n "$$tmp_dir" ]; then rm -rf "$$tmp_dir"; fi; \
	}; \
	trap cleanup 0; \
	trap 'exit 1' 1 2 3 15; \
	stage="$$tmp_dir/$$name"; \
	mkdir "$$stage"; \
	for entry in $(DIST_SRC_FILES); do \
		case "$$entry" in \
			''|/*|..|../*|*/..|*/../*|./*|*/./*|*\\*) \
				echo "dist.src: invalid source path: $$entry" >&2; exit 1 ;; \
		esac; \
		if [ ! -f "$$source_dir/$$entry" ] || [ -L "$$source_dir/$$entry" ]; then \
			echo "dist.src: missing or symlinked source file: $$entry" >&2; exit 1; \
		fi; \
		entry_dir=$$(dirname "$$entry"); \
		if [ "$$entry_dir" != '.' ]; then mkdir -p "$$stage/$$entry_dir"; fi; \
		cp "$$source_dir/$$entry" "$$stage/$$entry"; \
	done; \
	chmod +x "$$stage/configure"; \
	tmp_archive=$$(mktemp "$$out_dir/.rogue36-src.XXXXXXXX"); \
	( cd "$$tmp_dir" && tar -czf "$$tmp_archive" "$$name" ); \
	ln "$$tmp_archive" "$$output"; \
	echo "dist.src: created $$output"
