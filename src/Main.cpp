#include <Windows.h>

#include <memory>

#include "App.h"
#include "Window/MainWindow.h"

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    UNREFERENCED_PARAMETER(hInstance);
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);
    UNREFERENCED_PARAMETER(nCmdShow);

    App* app;
    if (!App::Create(app)) {
        MessageBox(nullptr, L"Failed to create application", L"Error", MB_OK | MB_ICONERROR);
        return -1;
    }

    // Creates the main window.
    MainWindow* mainWindow;
    if (!MainWindow::Create(app, mainWindow)) {
        MessageBox(nullptr, L"Failed to create main window", L"Error", MB_OK | MB_ICONERROR);
        return -1;
    }

    return mainWindow->Run();
}