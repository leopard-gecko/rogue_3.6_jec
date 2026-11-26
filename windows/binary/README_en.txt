Rogue 3.6 JEC  Windows Edition
===============================

Rogue is a computer role-playing game that originated around 1980.
Players explore a dungeon depicted with text and symbols, fight monsters,
and use various items in search of the Amulet of Yendor hidden deep
within the dungeon. It is the original "roguelike" game.

Rogue 3.6 is the oldest version of Rogue currently available, and can
be regarded as the starting point of Rogue. It retains features unique
to early Rogue, including monsters, items, and game mechanics that differ
from those in the final version, Rogue 5.4.

This package contains Rogue 3.6 JEC for Windows 10 / 11 (64-bit).
It respects the original Rogue 3.6 gameplay as closely as possible,
while adding Japanese/English language switching, Japanese and emoji
input/output, compatibility with modern Windows, and improvements
to the display and save handling.

Enjoy Rogue where it all began.

[Included Files]

  rogue36.exe       Game executable
  rogue_ja.html     Guide to the Dungeons of Doom (Japanese HTML)
  rogue_ja.txt      Guide to the Dungeons of Doom (Japanese TXT)
  rogue_en.html     Guide to the Dungeons of Doom (English HTML)
  rogue_en.txt      Guide to the Dungeons of Doom (English TXT)
  README_ja.txt     Japanese README
  README_en.txt     This file
  LICENSE.txt       Copyright and license information

[Starting the Game]

1. Extract the ZIP archive into a folder of your choice.

2. To start from File Explorer, double-click rogue36.exe.

   The following menu appears only if the default save file
   (rogue36.save) exists:

     [R/Enter] Resume the saved adventure
     [N]       Start a new adventure
     [F]       Choose another save file
     [Q]       Quit

   If no default save exists, a new adventure begins without a menu.
   [F] opens the Windows file picker; cancelling returns to the menu.
   Selecting [N] leaves existing save files untouched. If restoring
   fails, the error stays visible until you press Enter.

3. Drag and drop a save file onto the rogue36.exe icon to resume from
   that file directly. Japanese characters in file and folder names
   are supported.

4. To start from a terminal, open the extracted folder. On Windows 11,
   right-click and select "Open in Terminal". If this option is not
   available, type powershell in File Explorer's address bar and
   press Enter.

       .\rogue36.exe

   You can also start from Command Prompt (cmd.exe). Starting without
   an argument from an already open terminal begins a new game without
   displaying the launch menu.

The game display is fixed at 80 columns by 24 rows. Use a terminal of
at least 80x24. When launched by double-clicking, the game attempts
to resize the Windows Terminal or Console Host window to 80x24, but
this may not work in every display environment. When launched from an
existing terminal, the window size is not changed.

For a double-click launch, the game clears the dungeon before showing
the scores at game over and keeps the window open until you press Enter.
When launched from an existing terminal, it does not wait at exit.

[Save Files and Scores]

The default filenames are:

  Save file         rogue36.save
  Score file        rogue36.scr

These files are normally created in the working directory at startup.
When started directly from File Explorer, that is usually the folder
containing the executable. These filenames are the defaults for a
program built under the name rogue36; renaming the executable later
does not automatically change them.

To resume the default save file from a terminal:

       .\rogue36.exe rogue36.save

To specify a save file with a different name or location:

       .\rogue36.exe "C:\Games\Rogue36\my-save.save"

To display the score list only:

       .\rogue36.exe -s

A save file is automatically deleted when it is successfully restored.
To keep it, make a copy before resuming. If file= is specified in
ROGUEOPTS, that setting takes precedence over the default save location.

This binary distribution does not automatically save to the Windows
Saved Games folder. Personal builds from source can use
FILEDIR=saved-games, but this embeds the builder's absolute directory
in the EXE and is not suitable for redistribution.

[Setting Environment Variables in Windows]

The ROGUEOPTS environment variable can be used for startup settings.
Set language=ja for Japanese or language=en for English. For the
meanings of other options, see "9. Options" in the included English
guide (rogue_en.html). This section explains how to configure the
environment variable on Windows.

To set it temporarily in PowerShell:

       $env:ROGUEOPTS = "name=Hero,language=en"
       .\rogue36.exe

In Command Prompt (cmd.exe):

       set "ROGUEOPTS=name=Hero,language=en"
       rogue36.exe

These settings apply to the current terminal and to programs started
from it. They are not inherited by a separate double-click launch
from File Explorer.

To apply the settings to double-click launches, add ROGUEOPTS to your
Windows user environment variables:

  1. Press Win + R, type sysdm.cpl, and run it.
  2. Select "Advanced" -> "Environment Variables".
  3. Under "User variables", select "New" (or "Edit" if it exists).
  4. Enter ROGUEOPTS as the variable name and your settings as its value.
  5. Save with "OK". Sign out of Windows and back in if needed.

[Game Guide and License]

For instructions, screen information, commands, items, and options,
open the included rogue_en.html ("A Guide to the Dungeons of Doom")
in a web browser, or read the TXT edition (rogue_en.txt).
Neither version requires an Internet connection.

The latest guide is also available on the following website:

  https://leopard-gecko.github.io/jrogue/guide.html?version=36

An Internet connection is required to view the website.
For copyright and redistribution terms, see the included LICENSE.txt.

Source code and detailed platform-specific READMEs are available at:

  https://github.com/leopard-gecko/rogue_3.6_jec

MSYS2 is not required simply to run this binary distribution.
