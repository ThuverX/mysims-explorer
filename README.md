# MySims Explorer

A tool to easily browse, inspect and view game assets from **MySims** and **MySims Kingdom** (Cozy Bundle edition).

![Screenshot of MySims Explorer](https://raw.githubusercontent.com/bottledlactose/mysims-explorer/refs/heads/trunk/images/screenshot.png)

## Features

- **Explore Game Assets**  
   Easily browse, search and inspect raw asset files from MySims and MySims Kingdom (Cozy Bundle edition).

- **Real-Time 3D Model Viewer**  
   Visualize 3D models with live material previews in a responsive viewport.

- **Material Preview Support**  
  Toggle between different material variants for models (e.g., facial expressions, clothing variations).

- **Detailed Property Inspector**  
  Inspect metadata such as meshes, materials, bounding boxes, and additional asset details.

- **Integrated Console Panel**  
  View live log messages and asset loading traces in a built-in console.

- **Asset Map & Filename Unhashing**  
   Automatically reveals known asset names and types via partial unhashing when available.

- **Lightweight, Classic Desktop UI**  
   Features panels for Explorer, Viewport, Properties, and Console, optimized for efficient, smooth inspection.

- **Cross-Platform**  
  Compatible with 64-bit systems running Windows and Linux.

## Usage

1. Download the latest version on the [Releases](https://github.com/bottledlactose/mysims-explorer/releases) page
2. Launch the downloaded executable file
3. Navigate to `File` -> `Open Data Root`
4. Select the data directory for your game through the dropdown or select a custom path
5. Browse and explore the game assets through the explorer panel

If you run into any issues or if you'd like to share some feedback, feel free to open an issue
on the [Issues](https://github.com/bottledlactose/mysims-explorer/issues) page or join the
[MySims Clubhouse](https://discord.com/invite/2j9mU6zNJH) Discord server!

### Controls

The viewport camera is controlled through the left and right mouse buttons:

- **Left Mouse Button** - Rotate the camera around the subject
- **Right Mouse Button** - Pan the camera to move around

## Building from Source

### Windows

1. Clone the repository:
    - `git clone --recurse-submodules https://github.com/bottledlactose/mysims-explorer.git`
2. Open the cloned folder in your preferred IDE
3. Build and run using MSVC or Clang-cl (recommended).

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

![Screenshot #1](https://raw.githubusercontent.com/bottledlactose/mysims-explorer/refs/heads/trunk/images/screenshot1.png)
![Screenshot #2](https://raw.githubusercontent.com/bottledlactose/mysims-explorer/refs/heads/trunk/images/screenshot2.png)
![Screenshot #3](https://raw.githubusercontent.com/bottledlactose/mysims-explorer/refs/heads/trunk/images/screenshot3.png)
![Screenshot #4](https://raw.githubusercontent.com/bottledlactose/mysims-explorer/refs/heads/trunk/images/screenshot4.png)

## License

This project is licensed under the [MIT License](https://github.com/bottledlactose/mysims-explorer/blob/trunk/LICENSE).
