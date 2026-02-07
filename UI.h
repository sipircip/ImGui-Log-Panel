#pragma once
#include "pch.h"

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND, UINT, WPARAM, LPARAM);

class UI
{
public:
    static int Run();              
    static void RequestExit();   

private:
    static bool CreateDeviceD3D(HWND hWnd);
    static void CleanupDeviceD3D();
    static void CreateRenderTarget();
    static void CleanupRenderTarget();
    static LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

private:
    static inline HWND                 g_hWnd = nullptr;
    static inline bool                 g_done = false;

    static inline ID3D11Device* g_pd3dDevice = nullptr;
    static inline ID3D11DeviceContext* g_pd3dDeviceContext = nullptr;
    static inline IDXGISwapChain* g_pSwapChain = nullptr;
    static inline ID3D11RenderTargetView* g_mainRTV = nullptr;
};
