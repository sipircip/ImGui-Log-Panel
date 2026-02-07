#include "pch.h"
#include "Drawing.h"
#include "UI.h"

#include "LogPanel.h"
#include "Logger.h"

static bool inited = false;

void Drawing::Draw()
{
    static bool g_open = true;
    // Toggle the window with the INSERT key.
    if (GetAsyncKeyState(VK_INSERT) & 1)
        g_open = !g_open;

    // One-time init (run once, not every frame)
   
    if (!inited) {
        LogPanel::Init();
        LogPanel::PushDemo();  
        inited = true;
    }

    if (g_open) {
        ImGui::Begin("Log Panel", &g_open);

        LogPanel::PanelDraw();

        // Examples below show how to log dynamic messages (strings, numbers, and composed text).
        // --- Examples: logging dynamic strings (std::string / numbers) ---

        if (ImGui::Button("LOG string concat"))
        {
            std::string ip = "127.0.0.1";
            int port = 9999;

            // std::string concat example
            LOGI("net", std::string("Connecting to ") + ip + ":" + std::to_string(port));
        }
        ImGui::SameLine();

        if (ImGui::Button("LOG std::to_string"))
        {
            int seq_from = 120;
            int seq_to = 124;

            LOGW("udp", std::string("RX_GAP detected seq=") + std::to_string(seq_from) +
                " -> " + std::to_string(seq_to));
        }
        ImGui::SameLine();

        if (ImGui::Button("LOG build message"))
        {
            std::string user = "sipir";
            bool ok = false;

            // building a message based on a condition
            LOGE("auth", std::string("Login for '") + user + "' => " + (ok ? "OK" : "FAILED"));
        }

        if (ImGui::Button("LOG ostringstream"))
        {
            std::ostringstream oss;
            oss << "Packet stats: loss=" << 3 << " dup=" << 1 << " rtt=" << 28 << "ms";
            LOGI("stats", oss.str());
        }


        ImGui::Text("FPS: %.1f", ImGui::GetIO().Framerate);
        ImGui::End();
    }
}
