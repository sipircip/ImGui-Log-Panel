// src/main.cpp
#include "pch.h"
#include "UI.h"

// -------------------- Entry --------------------
//int WINAPI wWinMain(HINSTANCE, HINSTANCE, PWSTR, int)
int WINAPI wWinMain(_In_ HINSTANCE hInstance,
    _In_opt_ HINSTANCE hPrevInstance,
    _In_ LPWSTR lpCmdLine,
    _In_ int nShowCmd)
{
    return UI::Run();
}
