# Development Steps


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

## Main entry point (git:windows/00-main)
* Implement the standard `WinMain(HINSTANCE, HINSTANCE, LPSTR, int)` handler.
    1. `WinMain` is the entry point for Windows applications, similar to `main` in console applications.
    2. Create the main window instance and enter the main application loop.
