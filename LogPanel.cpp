#include "LogPanel.h"
#include "Logger.h"

#include <windows.h>
#include <unordered_map>
#include <vector>
#include <string>

#include "imgui/imgui.h"

namespace
{
    struct UiSettings {
        std::unordered_map<std::string, bool> cat_enabled;
        bool show_info = true;
        bool show_warn = true;
        bool show_error = true;
        char text_filter[128] = "";
    };

    UiSettings g_ui;

    static std::string settings_path()
    {
        char buf[MAX_PATH]{};
        GetModuleFileNameA(nullptr, buf, MAX_PATH);
        std::string exe = buf;
        auto pos = exe.find_last_of("\\/");
        std::string dir = (pos == std::string::npos) ? "." : exe.substr(0, pos);
        return dir + "\\logpanel_settings.ini";
    }

    static void settings_load(UiSettings& s)
    {
        FILE* f = nullptr;
        fopen_s(&f, settings_path().c_str(), "rb");
        if (!f) return;

        char line[512];
        while (fgets(line, sizeof(line), f)) {
            std::string l = line;
            while (!l.empty() && (l.back() == '\n' || l.back() == '\r')) l.pop_back();

            auto eq = l.find('=');
            if (eq == std::string::npos) continue;
            std::string k = l.substr(0, eq);
            std::string v = l.substr(eq + 1);

            auto asBool = [&](const std::string& x) { return x == "1" || x == "true" || x == "True"; };

            if (k == "show_info")  s.show_info = asBool(v);
            else if (k == "show_warn")  s.show_warn = asBool(v);
            else if (k == "show_error") s.show_error = asBool(v);
            else if (k == "text_filter") strncpy_s(s.text_filter, v.c_str(), sizeof(s.text_filter) - 1);
            else if (k.rfind("cat.", 0) == 0) s.cat_enabled[k.substr(4)] = asBool(v);
        }

        fclose(f);
    }

    static void settings_save(const UiSettings& s)
    {
        FILE* f = nullptr;
        fopen_s(&f, settings_path().c_str(), "wb");
        if (!f) return;

        fprintf(f, "show_info=%d\n", s.show_info ? 1 : 0);
        fprintf(f, "show_warn=%d\n", s.show_warn ? 1 : 0);
        fprintf(f, "show_error=%d\n", s.show_error ? 1 : 0);
        fprintf(f, "text_filter=%s\n", s.text_filter);

        for (const auto& [cat, en] : s.cat_enabled)
            fprintf(f, "cat.%s=%d\n", cat.c_str(), en ? 1 : 0);

        fclose(f);
    }

    static const char* level_name(LogLevel l)
    {
        switch (l) {
        case LogLevel::Info:  return "INFO";
        case LogLevel::Warn:  return "WARN";
        case LogLevel::Error: return "ERR";
        default:              return "?";
        }
    }

    static bool level_enabled(LogLevel l, const UiSettings& s)
    {
        if (l == LogLevel::Info) return s.show_info;
        if (l == LogLevel::Warn) return s.show_warn;
        return s.show_error;
    }

    static bool cat_enabled(const std::string& cat, UiSettings& s)
    {
        auto it = s.cat_enabled.find(cat);
        if (it == s.cat_enabled.end()) {
            s.cat_enabled[cat] = true; // default ON
            return true;
        }
        return it->second;
    }

    static bool text_match(const char* filter, const std::string& msg)
    {
        if (!filter || !filter[0]) return true;
        return msg.find(filter) != std::string::npos;
    }
}

namespace LogPanel
{
    void Init()
    {
        settings_load(g_ui);
    }

    void PushDemo()
    {
        auto& log = AppLogger();
        log.push(LogLevel::Info, "net", "Client started");
        log.push(LogLevel::Warn, "udp", "RX_GAP detected seq=120 -> 124");
        log.push(LogLevel::Error, "aes", "Decrypt failed (bad tag)");
        log.push(LogLevel::Info, "spam", "Packet spam example 1");
        log.push(LogLevel::Info, "spam", "Packet spam example 2");
    }

    void PanelDraw()
    {
        auto& log = AppLogger();

        static std::vector<LogItem> snap;
        static std::vector<std::string> cats;

        // Take snapshot only when new logs arrive
        const bool snap_updated = log.dirty();
        if (snap_updated)
            log.snapshot(snap);

        bool changed = false;

        changed |= ImGui::Checkbox("INFO", &g_ui.show_info);  ImGui::SameLine();
        changed |= ImGui::Checkbox("WARN", &g_ui.show_warn);  ImGui::SameLine();
        changed |= ImGui::Checkbox("ERROR", &g_ui.show_error);

        changed |= ImGui::InputText("Filter", g_ui.text_filter, IM_ARRAYSIZE(g_ui.text_filter));
        ImGui::Separator();

        // Rebuild category list only when snapshot changed
        if (snap_updated)
        {
            cats.clear();
            std::unordered_map<std::string, bool> seen;
            for (auto& it : snap)
                if (seen.emplace(it.category, true).second)
                    cats.push_back(it.category);
        }

        if (!cats.empty()) {
            ImGui::Text("Categories:");
            for (auto& c : cats) {
                bool en = cat_enabled(c, g_ui);
                if (ImGui::Checkbox(c.c_str(), &en)) {
                    g_ui.cat_enabled[c] = en;
                    changed = true;
                }
            }
            ImGui::Separator();
        }

        ImGui::BeginChild("logscroll", ImVec2(0, 500), true);

        ImGuiListClipper clipper;
        clipper.Begin((int)snap.size());
        while (clipper.Step())
        {
            for (int i = clipper.DisplayStart; i < clipper.DisplayEnd; i++)
            {
                const auto& it = snap[i];

                if (!level_enabled(it.level, g_ui)) continue;
                if (!cat_enabled(it.category, g_ui)) continue;
                if (!text_match(g_ui.text_filter, it.text)) continue;

                ImGui::Text("[%s] (%s) %s",
                    level_name(it.level),
                    it.category.c_str(),
                    it.text.c_str());
            }
        }

        ImGui::EndChild();

        if (changed)
            settings_save(g_ui);
    }


}
