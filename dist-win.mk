# Windows binary distribution (MSYS2 UCRT64): match the v004 release layout.
# The README and HTML guides are the original end-user documents from the
# published GitHub Release; source HTML and platform READMEs stay untouched.
DIST_WIN_NAME ?= rogue36_004_windows
DIST_WIN_FILE ?= $(DIST_WIN_NAME).zip

# make dist.win uses rogue36.exe by default even though plain make builds
# rogue.exe.  Explicit PROG= has priority over PROGRAM=, as elsewhere.
ifeq ($(origin PROG),command line)
DIST_WIN_PROGRAM := $(PROG)
else ifneq ($(strip $(PROGRAM)),)
DIST_WIN_PROGRAM := $(PROGRAM)
else
DIST_WIN_PROGRAM := rogue36
endif

.PHONY: dist.win
dist.win:
	@command -v zip >/dev/null 2>&1 || { \
		echo 'dist.win: zip is required (pacman -S --needed zip)' >&2; exit 1; \
	}
	@set -eu; \
	program=$(call shell_quote,$(DIST_WIN_PROGRAM)); \
	output=$(call shell_quote,$(DIST_WIN_FILE)); \
	case "$$program" in \
		''|.|..|-*|*[!a-zA-Z0-9._-]*|*.exe) \
			echo "dist.win: invalid executable name: $$program" >&2; exit 1 ;; \
	esac; \
	case "$$output" in \
		''|*/) echo 'dist.win: invalid ZIP filename' >&2; exit 1 ;; \
	esac; \
	if [ -n "$(strip $(FILEDIR))" ] || \
	   { [ "$(SAVEDIR)" != "." ] && [ -n "$(SAVEDIR)" ]; } || \
	   { [ "$(SCOREDIR)" != "." ] && [ -n "$(SCOREDIR)" ]; } || \
	   { [ "$(LOCKDIR)" != "." ] && [ -n "$(LOCKDIR)" ]; }; then \
		echo 'dist.win: FILEDIR and non-default save/score/lock paths are not allowed in portable binaries' >&2; \
		exit 1; \
	fi; \
	output_dir=$$(dirname "$$output"); \
	output_base=$$(basename "$$output"); \
	output_dir=$$(CDPATH= cd "$$output_dir" && pwd -P) || { \
		echo 'dist.win: output directory does not exist' >&2; exit 1; \
	}; \
	output="$$output_dir/$$output_base"; \
	if [ -e "$$output" ] || [ -L "$$output" ]; then \
		echo "dist.win: ZIP already exists (not overwritten): $$output" >&2; exit 1; \
	fi; \
	for guide in README_ja.txt README_en.txt rogue_ja.html rogue_en.html; do \
		if [ ! -f "windows/binary/$$guide" ] || [ -L "windows/binary/$$guide" ]; then \
			echo "dist.win: missing release document: $$guide" >&2; exit 1; \
		fi; \
	done; \
	if [ ! -f LICENSE.txt ] || [ -L LICENSE.txt ]; then \
		echo 'dist.win: LICENSE.txt is missing or a symbolic link' >&2; exit 1; \
	fi; \
	$(MAKE) -f Makefile.win all PROG="$$program" \
		FILEDIR= SAVEDIR=. SCOREDIR=. LOCKDIR=. \
		SAVEFILE="$$program.save" SCOREFILE="$$program.scr" \
		LOCKFILE="$$program.lock"; \
	if [ ! -f "$$program.exe" ]; then \
		echo "dist.win: executable not found: $$program.exe" >&2; exit 1; \
	fi; \
	COPYFILE_DISABLE=1; export COPYFILE_DISABLE; \
	stage_tmp=$$(mktemp -d "$${TMPDIR:-/tmp}/rogue36-win.XXXXXXXX"); \
	output_tmp=$$(mktemp -d "$$output_dir/.rogue36-win.XXXXXXXX"); \
	trap 'rm -rf "$$stage_tmp" "$$output_tmp"' 0; \
	trap 'exit 1' 1 2 3 15; \
	stage="$$stage_tmp/Rogue 3.6"; \
	mkdir -p "$$stage"; \
	cp "$$program.exe" "$$stage/$$program.exe"; \
	cp LICENSE.txt "$$stage/LICENSE.txt"; \
	for guide in README_ja.txt README_en.txt rogue_ja.html rogue_en.html; do \
		if [ "$$program" = rogue36 ]; then \
			cp "windows/binary/$$guide" "$$stage/$$guide"; \
		else \
			sed "s/rogue36/$$program/g" "windows/binary/$$guide" > "$$stage/$$guide"; \
		fi; \
	done; \
	(cd "$$stage_tmp" && zip -q -r "$$output_tmp/archive.zip" 'Rogue 3.6'); \
	ln "$$output_tmp/archive.zip" "$$output"; \
	echo "dist.win: created $$output"
