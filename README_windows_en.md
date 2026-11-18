# Rogue 3.6 — Windows (English)

This document explains how to build and run Rogue 3.6 JEC on **Windows 10 / 11**, including environment variables and save, score, and lock file locations. For an introduction to Rogue 3.6 and JEC, and links to all platform guides, see the [repository README](README.md). For gameplay, commands, items, and options, see the included [*A Guide to the Dungeons of Doom*](rogue_en.html).

日本語版：[README_windows_ja.md](README_windows_ja.md)

## 1. Prepare the build environment

Use **MSYS2 UCRT64** to build the Windows version. Install MSYS2 from <https://www.msys2.org/> and open "MSYS2 UCRT64" from the Start menu.

```sh
pacman -Syu
```

If an update requires restarting the terminal, reopen UCRT64 and finish updating. Then install the build packages:

```sh
pacman -S --needed make mingw-w64-ucrt-x86_64-gcc mingw-w64-ucrt-x86_64-binutils mingw-w64-ucrt-x86_64-pdcurses
```

Extract the source and change to its directory in UCRT64.

## 2. Build

```sh
make clean
make
```

This creates **`rogue.exe`**. Windows builds use VT rendering. You do not need `./configure` or `make install` on Windows.

| Purpose | Command |
|---|---|
| Regular build | `make` |
| Create `rogue_original.exe` with English as the initial language | `make PROGRAM=rogue_original original` |
| Include wizard support | `make wizard` |
| Name the executable `rogue36.exe` | `make PROGRAM=rogue36` |

`PROG=` remains supported. If both `PROGRAM=` and `PROG=` are specified, `PROG=` takes precedence. `make original` changes the initial language to English; it does not change gameplay or other defaults. Run `make clean` before rebuilding when changing the build configuration as needed.

## 3. Launch and exit

Double-click **`rogue.exe`** in Explorer. **If no default save file exists, a new game starts immediately without a menu.** Only when a default save file exists does the launch menu appear:

| Key | Action |
|---|---|
| `R` / `Enter` | Resume the default saved game |
| `N` | Start a new adventure |
| `F` | Select another save file using the Windows file picker |
| `Q` | Quit without starting |

To resume a different save even when there is no default save, **drag the save file onto the `rogue.exe` icon** in Explorer. This also supports folder and file names containing Japanese characters. Cancelling the file picker returns to the menu. Starting a new adventure leaves existing save files untouched. If restoring fails, an error remains visible until you press Enter.

Launching from PowerShell, Command Prompt or MSYS2 UCRT64 keeps the existing behavior: no launch menu, and a new game starts directly.

```powershell
.\rogue.exe
```

To resume from a specific save file when using a command shell, supply its path (for example, `./rogue.exe rogue.save` in MSYS2 or `.\rogue.exe rogue.save` in PowerShell).

The dungeon display is **fixed at 80 columns × 24 rows**. A direct double-click launch attempts to resize Windows Terminal or Console Host to 80×24. At game over, it clears the dungeon before showing the scores and keeps the window open until you press Enter. Launching from an existing shell does not resize that shell or wait for Enter on exit.

To display only the high-score list from PowerShell:

```powershell
.\rogue.exe -s
```

See the guide linked above for gameplay instructions.

## 4. Setting environment variables on Windows

Startup settings can be supplied through **`ROGUEOPTS`**. This section explains how to set the variable on Windows; for the meanings of the settings, see "Options" in the [English guide](rogue_en.html).

**Set a value temporarily in the current terminal:** `terse` is only an example value.

PowerShell:

```powershell
$env:ROGUEOPTS = 'terse'
.\rogue.exe
```

Command Prompt:

```cmd
set "ROGUEOPTS=terse"
rogue.exe
```

MSYS2 UCRT64:

```sh
export ROGUEOPTS='terse'
./rogue.exe
```

These settings apply to the current terminal and programs launched from it. They disappear when the terminal session ends.

**Make the setting available to double-click launches:**

1. Press `Win + R`, enter `sysdm.cpl` and press Enter.
2. Open **Advanced → Environment Variables**.
3. Under **User variables**, choose **New** or **Edit**. Set the variable name to `ROGUEOPTS` and enter the desired value.
4. Confirm with **OK**. If the new value is not applied, sign out of Windows and sign back in.

Remove `ROGUEOPTS` from the same dialog if it is no longer needed. File locations such as `FILEDIR` are build-time settings, not environment variables for the game.

## 5. Save, score and lock file locations

By default, **`rogue.save`**, **`rogue.scr`** and **`rogue.lock`** are placed in the game's working directory. Changing the executable name also changes the default data filenames.

**Keep all three files in one directory:**

```sh
mkdir -p rogue-data
make clean
make FILEDIR=rogue-data
```

Normal `FILEDIR` paths are **not created automatically**. Create the directory first and ensure the game can write to it. To store files elsewhere, for example:

```sh
mkdir -p '/c/Users/Public/Rogue36Data'
make clean
make FILEDIR='C:/Users/Public/Rogue36Data'
```

Alternatively, use `make prepare-filedir FILEDIR='C:/Users/Public/Rogue36Data'` to create the directory explicitly.

**Use the Windows Saved Games known folder (personal builds only):**

```sh
make clean
make FILEDIR=saved-games
```

This resolves the current user's actual Windows Saved Games folder (including redirection) and saves the game, score and lock files in its `Rogue/3.6` subfolder. This keeps Rogue 3.6 save and score files separate from Rogue 5.4.5. The directory is created automatically if necessary. **MSYS2 UCRT64 and PowerShell** are required for this option.

**Important: The builder's absolute directory path is embedded in the EXE. Do not use `FILEDIR=saved-games` when building binaries for distribution to other PCs; leave `FILEDIR` unset instead.**

**Use separate directories:**

```sh
mkdir -p rogue-data/saves rogue-data/scores
make clean
make SAVEDIR=rogue-data/saves \
     SCOREDIR=rogue-data/scores \
     LOCKDIR=rogue-data/scores
```

Set filenames individually with `SAVEFILE=NAME`, `SCOREFILE=NAME` and `LOCKFILE=NAME`. The directory overrides `SAVEDIR=DIR`, `SCOREDIR=DIR` and `LOCKDIR=DIR` take precedence over `FILEDIR` for their respective files. Rebuild after changing any of these settings. Relative paths are resolved from the game's working directory.

## 6. Files and distribution

| File | Purpose |
|---|---|
| `rogue.exe` | Built game executable |
| `rogue.save` / `rogue.scr` / `rogue.lock` | Save / score / lock files |
| `Makefile` / `Makefile.win` | Build files |
| `dist-src.mk` | Source distribution file list and archive creation |
| `windows/rogue.ico` / `windows/rogue-icon.rc` | Windows executable icon |
| `windows/windows-saved-games.ps1` | Locate the Windows Saved Games folder and create its `Rogue/3.6` subfolder |
| `*.c` / `*.h` | Core game source files in the top-level directory |
| `jec/` | JEC-specific shared modules (language selection, Unicode text handling and screen helpers) |
| `windows/` | Windows-specific source files and icon resources |
| `original-notes/` | Historical supporting files from the original Rogue 3.6 (not used in normal builds) |
| `rogue_ja.html` / `rogue_en.html` | *A Guide to the Dungeons of Doom* (Japanese / English, available offline) |
| `rogue_ja.txt` / `rogue_en.txt` | Plain-text Japanese / English guides formatted for monospaced display |
| `README.md` | Project overview and links to platform READMEs and dungeon guides |
| `LICENSE.txt` | License |

MSYS2 is only needed to build the source, not merely to launch an already-built executable.

**Creating a source archive:** Run `make dist.src` to create `rogue36_004-src.tar.gz`, containing the source files, Japanese/English READMEs and guides, license, and Windows-specific files. This does not compile or run `make clean`, modify saves/scores/executables/configuration, or package build outputs or macOS metadata. An existing archive is never overwritten; use `make dist.src DIST_FILE=another-name.tar.gz` to choose a new filename.

## 7. Copyright and license

See [LICENSE.txt](LICENSE.txt) for copyright notices and license terms for the original Rogue and existing code, as well as JEC modifications and acknowledgments.
