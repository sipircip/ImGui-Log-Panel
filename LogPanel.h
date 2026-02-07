#pragma once
#include "pch.h"

class Logger;

namespace LogPanel
{
    // Must be called once (creates default state and loads settings).
    void Init();

    // Call every frame when you want to draw the panel.
    void PanelDraw();

    // Optional: push demo messages (good for README/demo).
    void PushDemo();
}
