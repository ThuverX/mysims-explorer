#pragma once

struct UIState {
    // The current theme setting
    bool mIsDarkTheme = true;

    // Window display options
    bool mShowExplorer = true;
    bool mShowProperties = true;
    bool mShowConsole = true;

    // Rendering options
    bool mWireframeMode = false;
    bool mShowBounds = false;

    // The current search query
    char mSearchQuery[256];
};
