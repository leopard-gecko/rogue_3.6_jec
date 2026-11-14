# Rogue 3.6 — Windows (English)

Rogue is a computer role-playing game created around 1980, in which players explore dungeons drawn with letters and symbols. They battle monsters and use various items while searching for the Amulet of Yendor hidden deep within the dungeon.

**Rogue 3.6, widely distributed in 1981, is the oldest version of Rogue still available today.** It is a historically valuable glimpse of Rogue in its earliest days. Compared with later versions such as Rogue 5.2 and 5.4, it retains distinctive early monsters, items, and game mechanics.

This updated edition preserves the original gameplay as closely as possible while adding Japanese/English support and compatibility with modern operating systems.

For an overview of the game, controls, items and how to play, see the included [English guide, *A Guide to the Dungeons of Doom*](rogue_en.html), or the [Japanese guide, 「【運命の洞窟】への案内書」](rogue_ja.html). Both can be opened in a web browser without an internet connection. The guide is also available [online](https://leopard-gecko.github.io/jrogue/guide.html?version=36). This README explains requirements, building, launching, Windows environment variables, file locations and distribution.

This guide is for **Windows 10 / 11**. Other guides: [Windows・Japanese](README_windows_ja.md) / [macOS / Linux・Japanese](README_mac_linux_ja.md) / [macOS / Linux・English](README_mac_linux_en.md).

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
| `Makefile` / `Makefile.win` / `dist-src.mk` | Build files |
| `windows/rogue.ico` / `windows/rogue-icon.rc` | Windows executable icon |
| `windows/windows-saved-games.ps1` | Locate the Windows Saved Games folder and create its `Rogue/3.6` subfolder |
| `*.c` / `*.h` | Core game source files in the top-level directory |
| `jec/` | JEC-specific shared modules (language selection, Unicode text handling and screen helpers) |
| `windows/` | Windows-specific source files and icon resources |
| `original-notes/` | Historical supporting files from the original Rogue 3.6 (not used in normal builds) |
| `rogue_ja.html` / `rogue_en.html` | *A Guide to the Dungeons of Doom* (Japanese / English, available offline) |
| `tools/` | Source archive script and file manifest |
| `LICENSE.txt` | License |
| `README_windows_ja.md` / `README_windows_en.md` | Windows READMEs (Japanese/English) |
| `README_mac_linux_ja.md` / `README_mac_linux_en.md` | macOS / Linux READMEs (Japanese/English) |

If you copy `rogue.exe` to another PC, include any required runtime DLLs. MSYS2 is only needed to build the source, not merely to launch an already-built executable.

**Creating a source archive:** Run `make dist.src` to create `rogue36_004-src.tar.gz`, containing the source files, Japanese/English READMEs and guides, license, and Windows-specific files. This does not compile or run `make clean`, modify saves/scores/executables/configuration, or package build outputs or macOS metadata. An existing archive is never overwritten; use `make dist.src DIST_FILE=another-name.tar.gz` to choose a new filename.

## 7. Main changes from the original Rogue 3.6

- **Windows support:** Build a native executable under MSYS2 UCRT64, including a Windows application icon.
- **Two languages:** Japanese messages, items, options and score display, with startup and in-game switching.
- **Japanese text:** Improved UTF-8 input and display for names, settings and emoji, and improved message display.
- **Terminal display:** The dungeon is fixed at 80×24, and direct launches attempt to resize the terminal window.
- **Score display:** A direct launch clears the game view and keeps the score list visible until Enter is pressed.
- **Files and reliability:** Configurable data locations and improvements to scores, locking and wizard-mode password entry.
