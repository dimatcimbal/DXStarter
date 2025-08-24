# Development Steps

## Graphics Context and Device (git:dx/01-graphics-context)

* Create GraphicContext class that encaps DX12 device, command queue, etc.
    1. Create DebugLayer class initializing it members with `D3D12GetDebugInterface`, `ID3D12Debug::EnableDebugLayer`
       and `DXGIGetDebugInterface1`. Customize the DebugLayer destructor to report any live objects remaining.
       * Ensure the DebugLayer instance gets destroyed last to report on remaining live objects.
    2. Create Device class that initializes DXGI factory and D3D12 device; and is a factory for other DX12 objects.
       * Update `CMakeLists.txt` to include DX dependencies (`d3d12`, `dxgi`, `dxguid`)
       * Create a factory method `Device::Create` which loops over the available adapters and selects the first one that matches the
         criteria (is hardware, supports the requested feature level, has the highest video memory).
    

## Main Window (git:window/00-main-window)

* Create an application window with the following steps:
    1. **Get the module handle**: Use `GetModuleHandle(NULL)` to retrieve the handle to the current module.
    2. **Describe the window class**: Fill out a `WNDCLASSEX` structure to define the window's properties.
        * Include a pointer to the window procedure (e.g., `DefWindowProc` or a custom implementation).
        * Specify the class name, cursor, and other attributes.
    3. **Register the window class**: Use `RegisterClassEx` to register the class with the system.
        * Check the return value to ensure the registration succeeded.
    4. **Create the window**: Call `CreateWindowEx` with the registered class name and desired styles.
        * Handle errors if the function returns `NULL`.
    5. **Implement the main loop**: Handle rendering and processing of window messages:
        * Use `GetMessage` to retrieve messages from the message queue.
            * Check the return value to detect when the application should exit.
        * Use `TranslateMessage` to process keyboard input.
        * Use `DispatchMessage` to send messages to the window procedure.
        * When the loop exits, use `msg.wParam` as the application's exit code.
    6. Define `NOMINMAX` and `UNICODE` in `CMakeLists.txt` with `target_compile_definitions` to exclude min and max
       macros and use wide-character versions of WinAPI functions in macros like OutputDebugString.

## Main entry point (git:windows/00-main)

* Implement the standard `WinMain(HINSTANCE, HINSTANCE, LPSTR, int)` handler.
    1. `WinMain` is the entry point for Windows applications, similar to `main` in console applications.
    2. Create the main window instance and enter the main application loop.
