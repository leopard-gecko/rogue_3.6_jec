# Shared source-only archive target for Unix and MSYS2 Windows builds.
# No compile, clean, or distclean step is performed.
DIST_NAME ?= rogue36_004
DIST_FILE ?= $(DIST_NAME)-src.tar.gz

# Quote arguments for a POSIX shell, including paths containing spaces.
dist_shell_quote = '$(subst ','"'"',$(1))'

.PHONY: dist.src
dist.src:
	@sh tools/make-dist-src.sh $(call dist_shell_quote,$(DIST_NAME)) $(call dist_shell_quote,$(DIST_FILE))
