CS213 Assignment 2 - Part 3
Vole Machine Simulator GUI

Student information
-------------------
Section: S(23,24)
Student ID(s): (20250252, 20250483, 20250777, 2025795)
Student name(s): (Kareeem Tamer, Zeyad Amr, Youssef Adly, Youssef Mamdouh)
Course instructor: DR. Mohammed El Ramly

Project description
-------------------
This project is a graphical simulator for the Vole machine. It allows you to
load or enter machine-code programs, inspect CPU registers and memory, and
execute instructions one step at a time or until the program halts.

Requirements
------------
Option A - Automatic build (no setup needed):
- Windows 10 or 11 (curl is required and ships with Windows 10 1803+)
- Internet connection (first run only)
- At Most 5 GB of free disk space
  (downloads about 2-3 GB: Qt 6.8.1, MinGW 13.1, CMake, Ninja)

Option B - Manual build (if Qt is already installed):
- Qt 6.5 or later, including the Qt Core and Widgets components
- A C++ compiler supported by your Qt installation (for example, MinGW or
  MSVC)
- CMake 3.19 or later

Build and run
-------------

Option A - Automatic (recommended if you do not have Qt)
1. Double-click build.bat (located next to CMakeLists.txt).
2. On the first run, the script asks for confirmation before downloading
   about 2-3 GB. Press Y to continue, or N to cancel (for example, if you
   prefer to use Qt Creator, see Option B).
3. Wait while the script runs. The first run can take 10-30 minutes because
   of the downloads. Later runs are much faster, since the downloaded tools
   are kept in the hidden .deps folder.
4. The script performs these steps automatically:
   [1/5] Checks the system: curl is available, enough free disk space, and
         an internet connection.
   [2/5] Downloads aqt.exe, the tool used to fetch Qt.
   [3/5] Downloads and installs Qt 6.8.1, MinGW 13.1, Ninja, and CMake into
         the .deps folder. Failed downloads are retried up to 3 times.
   [4/5] Configures and builds the project with CMake.
   [5/5] Runs windeployqt to copy the required DLLs next to the executable.
5. When the build succeeds, the program starts automatically. The executable
   is located at build\GUI.exe and can be run again at any time.
6. All build output is written to build_log.txt. If anything fails, the
   window stays open with an error message and the details are in that file.

Option B - Qt Creator (if Qt is already installed)
1. Open Qt Creator and select File > Open File or Project, then choose
   CMakeLists.txt.
2. Select a configured Qt 6.5-or-later desktop kit, allow CMake to configure
   the project, then build and run the GUI target.

Notes
-----
- Keep the project path free of spaces and non-English characters if
  possible.
- If the build fails, open build_log.txt for the full details.
- To force a clean rebuild, delete the build folder and run build.bat again.
- To remove everything the script downloaded, delete the .deps folder.
- Windows SmartScreen may warn about the executable because it is not signed.
  Click "More info" and then "Run anyway".

Using the simulator
-------------------
1. Start the GUI application.
2. In Program Editor, choose a sample program or enter Vole machine code.
   Alternatively, use Browse to select a text file containing the program.
3. Set Load Address and Start PC as needed. For a program loaded at address
   zero, set both values to 0.
4. Select Load Code into Machine for editor code, or Load File after browsing
   to a text file.
5. Use Step to execute one instruction, Run (Timed) to run with adjustable
   speed, or Run to Halt to execute until HALT.
6. Use Reset to reset the machine. The CPU, registers, memory, screen output,
   and execution trace are shown in the GUI. Registers and memory cells can be
   double-clicked to edit their values.

Program files contain hexadecimal bytes. Instructions are four hexadecimal
digits (two bytes), and comments may follow a semicolon. For example:

    2105 ; R1 <- 0x05
    2203 ; R2 <- 0x03
    5312 ; R3 <- R1 + R2
    33FF ; Send R3 to screen output
    C000 ; Halt

If needed, the load address and entry point can be specified in comments:

    ; start address: 00
    ; pc: 00

Public video link: [Paste the video URL here]