#include <Windows.h>
#include "Window/MainWindow.h"

#include <memory>

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    UNREFERENCED_PARAMETER(hInstance);
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);
    UNREFERENCED_PARAMETER(nCmdShow);

    // Creates the main window.
    std::unique_ptr<MainWindow> MainWindow;
    if (!MainWindow::Create(MainWindow)) {
        MessageBox(nullptr, L"Failed to create main window", L"Error", MB_OK | MB_ICONERROR);
        return -1;
    }

    return MainWindow->Run();
}