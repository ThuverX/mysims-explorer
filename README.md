# MySims Explorer

A tool to easily browse, inspect and view game assets from MySims and MySims Kingdom (Cozy Bundle edition).

![Screenshot of MySims Explorer](https://raw.githubusercontent.com/bottledlactose/mysims-explorer/refs/heads/trunk/images/screenshot.png)

This was mostly tested on Kingdom, so things are more likely to break on the original MySims.

## Features

- View and browse asset files from MySims and MySims Kingdom
- Explore 3D models and materials in a real-time viewport with camera controls
- Inspect meta data for model and material resources
- Asset map parsing to unhash some file names where possible
- Built-in logging and debug console

## Roadmap

- Add Explorer filtering and search
- Add icons to the Explorer and main menu bar
- Add option to find references to the specified asset (see which asset is using which)
- Port to Linux

## Usage

1. Download the latest version on the [Releases](https://github.com/bottledlactose/mysims-explorer/releases) page
2. Launch the tool
3. Navigate to `File` -> `Open Data Root...`
4. Select your MySims or MySims Kingdom data directory
    - On Steam, these are usually the following directories:
        - MySims Kingdom: `C:\Program Files (x86)\Steam\steamapps\common\MySims Kingdom\data`
        - MySims: `C:\Program Files (x86)\Steam\steamapps\common\MySims\data`
5. Browse through the files loaded in the Explorer

## Compiling

### Windows

1. Clone the repository:
    - `git clone --recurse-submodules https://github.com/bottledlactose/mysims-explorer.git`
2. Open the cloned folder in your editor of choice
3. Build and run the code with your compiler of choice. MSVC or Clang-cl are recommended.

### Linux

1. Ensure you have the correct dependencies installed:
    - GCC + CMake: `sudo apt install build-essential cmake`
    - X11 libraries: `sudo apt install libx11-dev libxext-dev libxrandr-dev libxcursor-dev libxi-dev libxinerama-dev`
    - GTK3 libraries (needed for NFD): `sudo apt install libgtk-3-dev`
2. Clone the repository and compile the code:
    - `git clone --recurse-submodules https://github.com/bottledlactose/mysims-explorer.git`
    - `cd mysims-explorer`
    - `cmake ..`
    - `make -j`
3. Run the compiled binary:
    - `./mysims-explorer`

## Gallery

<img width="425" alt="image" src="https://raw.githubusercontent.com/bottledlactose/mysims-explorer/refs/heads/trunk/images/screenshot1.png" />
<img width="425" alt="image" src="https://raw.githubusercontent.com/bottledlactose/mysims-explorer/refs/heads/trunk/images/screenshot2.png" />

## License

This project is licensed under the MIT license. See [LICENSE](https://github.com/bottledlactose/mysims-explorer/blob/trunk/LICENSE) for details.
