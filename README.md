# ImGui Log Panel (Win32 + D3D11) — Dirty Snapshot + Clipper

A lightweight **standalone ImGui log viewer** for Windows (Win32 + DirectX11).

This project demonstrates a practical pattern for real-time log UIs:
- **Thread-safe logger** (ring-buffer)
- **Dirty snapshot** (only copy logs when new data arrives)
- **ImGuiListClipper** (only render visible rows)

Hotkeys:
- **INSERT**: toggle log window
- **END**: exit application

---

## Features

- **Standalone Win32 + D3D11 app**
- **Ring-buffer logger** (keeps the last N logs)
- **Log level filters**: INFO / WARN / ERROR
- **Category toggles** (per category on/off)
- **Text filter** (substring match)
- **Settings persistence** (simple INI file next to the exe)
- **Optimized UI loop**
  - dirty snapshot to avoid unnecessary copies
  - clipper to avoid drawing thousands of lines every frame

---

## Why “Dirty Snapshot” ?

The logger can receive messages from anywhere (even other threads).
If the UI copies the entire log buffer every frame, performance drops as logs grow.

Instead, we:
1. Mark the logger as **dirty** on every `push()`
2. In the UI, only take a `snapshot()` when dirty is true

```cpp
void Logger::push(LogLevel lvl, std::string cat, std::string msg)
{
    std::scoped_lock lk(m_);
    if (items_.size() >= max_) items_.pop_front();
    items_.push_back({ lvl, std::move(cat), std::move(msg), now_ms() });
    dirty_ = true;
}

void Logger::snapshot(std::vector<LogItem>& out)
{
    std::scoped_lock lk(m_);
    out.assign(items_.begin(), items_.end());
    dirty_ = false;
}

bool Logger::dirty() const { return dirty_.load(); }
```
Why ImGuiListClipper ?

Rendering thousands of rows every frame is expensive. ImGuiListClipper draws only
the currently visible range.
```cpp
ImGuiListClipper clipper;
clipper.Begin((int)snap.size());
while (clipper.Step())
{
    for (int i = clipper.DisplayStart; i < clipper.DisplayEnd; i++)
    {
        const auto& it = snap[i];
        ImGui::Text("[%s] (%s) %s", level_name(it.level), it.category.c_str(), it.text.c_str());
    }
}

```

Logging examples
```cpp
LOGI("net", "Client started");

std::string ip = "127.0.0.1";
int port = 9999;
LOGI("net", std::string("Connecting to ") + ip + ":" + std::to_string(port));

```
