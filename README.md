# MySims Explorer

A tool to easily browse, inspect and view game assets from MySims and MySims Kingdom (Cozy Bundle edition).

This was mostly tested on Kingdom, so things are more likely to break on the original MySims.

## Features

- View and browse asset files from MySims and MySims Kingdom
- Explore 3D models and materials in a real-time viewport with camera controls
- Inspect meta data for model and material resources
- Asset map parsing to unhash some file names where possible
- Built-in logging and debug console

## Usage

1. Download the latest version on the Releases page
2. Launch the tool
3. Navigate to `File` -> `Open Data Root...`
4. Select your MySims or MySims Kingdom data directory
    - On Steam, these are usually the following directories:
        - MySims Kingdom: `C:\Program Files (x86)\Steam\steamapps\common\MySims Kingdom\data`
        - MySims: `C:\Program Files (x86)\Steam\steamapps\common\MySims\data`
5. Browse through the models loaded in the Explorer

## Known Issues

- Characters can't load materials on MySims
- Asset map translations are currently disabled for MySims
- Scrolling up doesn't work in the console window

## License

This project is licensed under the MIT license. See [LICENSE](https://github.com/bottledlactose/mysims-explorer/blob/trunk/LICENSE) for details.
