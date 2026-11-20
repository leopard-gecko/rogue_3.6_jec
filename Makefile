# Auto-select MSYS2 UCRT64 native Windows build, preserving the original
# configure-free macOS make and Linux/Raspberry Pi build paths.
ifeq ($(strip $(ROGUE_BUILD)),)
ifeq ($(MSYSTEM),UCRT64)
ROGUE_BUILD := windows
else ifeq ($(OS),Windows_NT)
ifeq ($(strip $(MSYSTEM)),)
ROGUE_BUILD := windows
else
ROGUE_BUILD := unix
endif
else
ROGUE_BUILD := unix
endif
endif
ifeq ($(ROGUE_BUILD),windows)
include Makefile.win
else ifeq ($(ROGUE_BUILD),unix)

CC ?= cc
PROG ?= rogue
FILEDIR ?=
SAVEFILE ?= $(PROG).save
SAVEDIR ?= $(if $(FILEDIR),$(FILEDIR),.)
SCOREFILE ?= $(PROG).scr
SCOREDIR ?= $(if $(FILEDIR),$(FILEDIR),.)
LOCKFILE ?= $(PROG).lock
LOCKDIR ?= $(if $(FILEDIR),$(FILEDIR),.)
CFLAGS ?= -std=gnu89 -Wall -Wextra -Werror
CPPFLAGS ?= -D_DEFAULT_SOURCE -D_XOPEN_SOURCE=600
EXTRA_CPPFLAGS ?=
DEFAULT_LANGUAGE ?= 0
LDLIBS ?= -lcurses
PREFIX ?= /usr/local
BINDIR ?= $(PREFIX)/bin
INSTALL ?= install

-include config.mk

# PROGRAM is an alternative name for PROG on the make command line.
# Keep an explicit PROG= authoritative if both variables are provided.
# Use override so PROGRAM= can replace PROG set by ./configure (config.mk).
ifneq ($(strip $(PROGRAM)),)
ifneq ($(origin PROG),command line)
override PROG := $(PROGRAM)
endif
endif

ifneq ($(strip $(DEFAULT_LANGUAGE)),)
LANGUAGE_CPPFLAGS = -DDEFAULT_LANGUAGE=$(DEFAULT_LANGUAGE)
endif

# Quote the entire -D argument so paths containing spaces survive the
# shell and the C compiler still receives a string-literal macro value.
# unix_shell_quote also handles apostrophes in Unix home directory names.
unix_shell_quote = '$(subst ','"'"',$(1))'
override CPPFLAGS += -I. \
    $(call unix_shell_quote,-DROGUE_SAVEFILE="$(SAVEFILE)") \
    $(call unix_shell_quote,-DROGUE_SAVEDIR="$(SAVEDIR)") \
    $(call unix_shell_quote,-DROGUE_SCOREFILE="$(SCOREFILE)") \
    $(call unix_shell_quote,-DROGUE_SCOREDIR="$(SCOREDIR)") \
    $(call unix_shell_quote,-DROGUE_LOCKFILE="$(LOCKFILE)") \
    $(call unix_shell_quote,-DROGUE_LOCKDIR="$(LOCKDIR)") \
    $(LANGUAGE_CPPFLAGS) $(EXTRA_CPPFLAGS)
SRCS = armor.c chase.c command.c daemon.c daemons.c fight.c init.c io.c list.c main.c misc.c monsters.c move.c new_level.c options.c pack.c passages.c potions.c rings.c rip.c rooms.c save.c scrolls.c sticks.c things.c weapons.c wizard.c vers.c globals.c getpass.c jec/language.c jec/unicode_text.c jec/screen_text.c
OBJS = $(SRCS:.c=.o)

all: $(PROG)

$(PROG): $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) $(LDLIBS) -o $@

# Recompile when the compiler or effective build settings change, including
# values from ./configure (config.mk) and overrides passed directly to make.
# Keep the stamp's timestamp unchanged when its contents are identical.
.PHONY: FORCE_UNIX_CONFIG
FORCE_UNIX_CONFIG:

.unix-build-config: FORCE_UNIX_CONFIG
	@printf '%s\n' \
		$(call unix_shell_quote,CC=$(CC)) \
		$(call unix_shell_quote,CFLAGS=$(CFLAGS)) \
		$(call unix_shell_quote,CPPFLAGS=$(CPPFLAGS)) \
		$(call unix_shell_quote,LDLIBS=$(LDLIBS)) > $@.tmp
	@if test -f $@ && cmp -s $@.tmp $@; then rm -f $@.tmp; else mv $@.tmp $@; fi

# config.mk is optional: plain "make" must work without running configure.
# All Unix objects depend on the shared headers and the effective settings.
$(OBJS): .unix-build-config $(wildcard config.mk) rogue.h mach_dep.h jec/language.h jec/unicode_text.h jec/unicode_output.h jec/screen_text.h

.PHONY: all clean distclean japanese english original wizard install uninstall


japanese:
	$(MAKE) all DEFAULT_LANGUAGE=1 EXTRA_CPPFLAGS="$(EXTRA_CPPFLAGS)"

english:
	$(MAKE) all DEFAULT_LANGUAGE=2 EXTRA_CPPFLAGS="$(EXTRA_CPPFLAGS)"

original:
	$(MAKE) all DEFAULT_LANGUAGE=2 EXTRA_CPPFLAGS="$(EXTRA_CPPFLAGS)"

wizard:
	$(MAKE) clean PROG="$(PROG)" FILEDIR="$(FILEDIR)" SAVEFILE="$(SAVEFILE)" SAVEDIR="$(SAVEDIR)" SCOREFILE="$(SCOREFILE)" SCOREDIR="$(SCOREDIR)" LOCKFILE="$(LOCKFILE)" LOCKDIR="$(LOCKDIR)"
	$(MAKE) PROG="$(PROG)" FILEDIR="$(FILEDIR)" SAVEFILE="$(SAVEFILE)" SAVEDIR="$(SAVEDIR)" SCOREFILE="$(SCOREFILE)" SCOREDIR="$(SCOREDIR)" LOCKFILE="$(LOCKFILE)" LOCKDIR="$(LOCKDIR)" EXTRA_CPPFLAGS="$(EXTRA_CPPFLAGS) -DWIZARD"

install: $(PROG)
	$(INSTALL) -d "$(DESTDIR)$(BINDIR)"
	@for d in "$(SAVEDIR)" "$(SCOREDIR)" "$(LOCKDIR)"; do \
		case "$$d" in ""|.) continue ;; esac; \
		case "$$d" in \
			'~') d="$${HOME:?HOME is required for ~ paths}" ;; \
			'~/'*) path=$${d#\~/}; d="$${HOME:?HOME is required for ~ paths}"; d="$${d%/}/$$path" ;; \
		esac; \
		case "$$d" in \
			/*) $(INSTALL) -d "$(DESTDIR)$$d" ;; \
			*)  $(INSTALL) -d "$$d" ;; \
		esac; \
	done
	$(INSTALL) -m 0755 "$(PROG)" "$(DESTDIR)$(BINDIR)/$(PROG)"

uninstall:
	rm -f "$(DESTDIR)$(BINDIR)/$(PROG)"

clean:
	rm -f $(OBJS) language.o unicode_text.o screen_text.o $(PROG) rogue .unix-build-config .unix-build-config.tmp .language-build-config .language-build-config.tmp

distclean: clean
	rm -f config.mk config.log conftest conftest.c

# Keep the source archive implementation identical across supported OSes.
include dist-src.mk

else
$(error ROGUE_BUILD must be windows or unix)
endif
