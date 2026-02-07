#include "UI.h"
#include "Drawing.h"


#ifndef WM_DPICHANGED
#define WM_DPICHANGED 0x02E0
#endif

void UI::RequestExit() { g_done = true; }

bool UI::CreateDeviceD3D(HWND hWnd)
{
    DXGI_SWAP_CHAIN_DESC sd{};
    sd.BufferCount = 2;
    sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    sd.BufferDesc.RefreshRate.Numerator = 60;
    sd.BufferDesc.RefreshRate.Denominator = 1;
    sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
    sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.OutputWindow = hWnd;
    sd.SampleDesc.Count = 1;
    sd.Windowed = TRUE;
    sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

    UINT createFlags = 0;
    D3D_FEATURE_LEVEL fl;
    const D3D_FEATURE_LEVEL fls[2] = { D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_0 };

    if (D3D11CreateDeviceAndSwapChain(
        nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr,
        createFlags, fls, 2, D3D11_SDK_VERSION,
        &sd, &g_pSwapChain, &g_pd3dDevice, &fl, &g_pd3dDeviceContext) != S_OK)
        return false;

    CreateRenderTarget();
    return true;
}

void UI::CreateRenderTarget()
{
    ID3D11Texture2D* backBuffer = nullptr;
    g_pSwapChain->GetBuffer(0, IID_PPV_ARGS(&backBuffer));
    if (backBuffer)
    {
        g_pd3dDevice->CreateRenderTargetView(backBuffer, nullptr, &g_mainRTV);
        backBuffer->Release();
    }
}

void UI::CleanupRenderTarget()
{
    if (g_mainRTV) { g_mainRTV->Release(); g_mainRTV = nullptr; }
}

void UI::CleanupDeviceD3D()
{
    CleanupRenderTarget();
    if (g_pSwapChain) { g_pSwapChain->Release(); g_pSwapChain = nullptr; }
    if (g_pd3dDeviceContext) { g_pd3dDeviceContext->Release(); g_pd3dDeviceContext = nullptr; }
    if (g_pd3dDevice) { g_pd3dDevice->Release(); g_pd3dDevice = nullptr; }
}

LRESULT WINAPI UI::WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    /*if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
        return true;*/

    if (ImGui::GetCurrentContext() &&
        ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
        return true;


    switch (msg)
    {
        case WM_SIZE:
            if (g_pd3dDevice && wParam != SIZE_MINIMIZED)
            {
                CleanupRenderTarget();
                g_pSwapChain->ResizeBuffers(0, (UINT)LOWORD(lParam), (UINT)HIWORD(lParam), DXGI_FORMAT_UNKNOWN, 0);
                CreateRenderTarget();
            }
            return 0;

        case WM_SYSCOMMAND:
            if ((wParam & 0xfff0) == SC_KEYMENU) 
                return 0;
            break;

        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;

        /*case WM_DPICHANGED:
            if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_DpiEnableScaleViewports)
            {
                const RECT* r = (const RECT*)lParam;
                SetWindowPos(hWnd, nullptr, r->left, r->top, r->right - r->left, r->bottom - r->top,
                    SWP_NOZORDER | SWP_NOACTIVATE);
            }
            break;*/
        case WM_DPICHANGED:
        {
            // ImGui context may not exist yet (window can receive messages before CreateContext()).
            if (ImGui::GetCurrentContext() && ImGui::GetIO().ConfigDpiScaleViewports)
            {
                const RECT* r = (const RECT*)lParam;
                SetWindowPos(hWnd, nullptr,
                    r->left, r->top,
                    r->right - r->left, r->bottom - r->top,
                    SWP_NOZORDER | SWP_NOACTIVATE);
                return 0;
            }
            break;
        }

    }
    return DefWindowProc(hWnd, msg, wParam, lParam);
}

int UI::Run()
{
    ImGui_ImplWin32_EnableDpiAwareness();

    WNDCLASSEX wc{ sizeof(WNDCLASSEX), CS_CLASSDC, WndProc, 0L, 0L, GetModuleHandle(nullptr),
                   nullptr, nullptr, nullptr, nullptr, L"ImGuiHost", nullptr };
    RegisterClassEx(&wc);

    
    g_hWnd = CreateWindow(wc.lpszClassName, L"Host",
        WS_OVERLAPPEDWINDOW, 100, 100, 50, 50,
        nullptr, nullptr, wc.hInstance, nullptr);

    if (!CreateDeviceD3D(g_hWnd))
    {
        CleanupDeviceD3D();
        UnregisterClass(wc.lpszClassName, wc.hInstance);
        return 1;
    }

    ShowWindow(g_hWnd, SW_HIDE);
    UpdateWindow(g_hWnd);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();

    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
    // ImGui 1.92.x DPI scaling options
    io.ConfigDpiScaleViewports = true;
    io.ConfigDpiScaleFonts = true; // optional but nice for high-DPI

    //io.ConfigFlags |= ImGuiConfigFlags_DpiEnableScaleViewports;

    ImGui::StyleColorsDark();

    io.IniFilename = "imgui_layout.ini";

    ImGui_ImplWin32_Init(g_hWnd);
    ImGui_ImplDX11_Init(g_pd3dDevice, g_pd3dDeviceContext);


    const ImVec4 clear(0.08f, 0.08f, 0.09f, 1.0f);

    g_done = false;
    while (!g_done)
    {
        MSG msg;
        while (PeekMessage(&msg, nullptr, 0U, 0U, PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
            if (msg.message == WM_QUIT) g_done = true;
        }
        if (g_done) break;

       
        // Exit the main loop when END is pressed.
        // "& 1" triggers only once per key press (not every frame while the key is held).
        if (GetAsyncKeyState(VK_END) & 1) g_done = true;

        ImGui_ImplDX11_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();

        Drawing::Draw();  

        ImGui::Render();

        const float clear_col[4] = { clear.x * clear.w, clear.y * clear.w, clear.z * clear.w, clear.w };
        g_pd3dDeviceContext->OMSetRenderTargets(1, &g_mainRTV, nullptr);
        g_pd3dDeviceContext->ClearRenderTargetView(g_mainRTV, clear_col);
        ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

        if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
        {
            ImGui::UpdatePlatformWindows();
            ImGui::RenderPlatformWindowsDefault();

            // Restore main render target (some backends change it)
            g_pd3dDeviceContext->OMSetRenderTargets(1, &g_mainRTV, nullptr);
        }

        // VSync frame pacing (FPS depends on monitor refresh rate):
        // 1 => ~RefreshRate / 1   (e.g. 144Hz -> ~144 FPS)
        // 2 => ~RefreshRate / 2   (e.g. 144Hz -> ~72 FPS)
        // 3 => ~RefreshRate / 3   (e.g. 144Hz -> ~48 FPS)
        g_pSwapChain->Present(3, 0);

    }

    ImGui_ImplDX11_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();

    CleanupDeviceD3D();
    DestroyWindow(g_hWnd);
    UnregisterClass(wc.lpszClassName, wc.hInstance);

    return 0;
}
