# Devlog


## Back Buffer (git:dx/06-back-buffer)
* As the `SwapChain` gets resized on `SwapChain::Resize` get references to the underlying buffers from the Swap Chain using `IDXGISwapChain3::GetBuffer`
    * Remember to free the references within `SwapChain::FlushAll` to avoid memory leaks.

## Descriptor Heaps (git:dx/06-descriptor-heaps)

* Add a new `DescriptorHeap` class that wraps the DirectX 12 descriptor heap `ID3D12DescriptorHeap`, keeping track of
  its type, size, count.
    1. Add a `CreateHandles` method that allocates descriptor handles from the heap, automatically tracks usage via both
       `DescriptorHeap::mCurrentHandle` value and `DescriptorHeap::mFreeDescriptorCount` counter, and reports errors if
       running out of descriptors occurs.
    2. Provide a `Device::CreateDescriptorHeap` method for creation of the `DescriptorHeap` object.
    3. Update the `GraphicsContext` constructor and initialization so it now supports and owns a descriptor heap.
    4. Verify that descriptor heap allocation works by testing handle creation during graphics context startup.

## Swap Chain (git:dx/05-swap-chain)

* Create SwapChain class that wraps a `IDXGISwapChain1`.
    1. Create a factory method `Device::CreateSwapChain` that initializes and returns a `SwapChain` object.
        * Describe the swap chain using a `DXGI_SWAP_CHAIN_DESC1` struct, specifying buffer count, format, usage, and
          swap effect.
        * Use `IDXGIFactory7::CreateSwapChainForHwnd` to create the swap chain for a window, passing the command queue,
          window handle, and swap chain description.
        * Disable the Alt+Enter fullscreen toggle feature with
          `IDXGIFactory4::MakeWindowAssociation(HWND, DXGI_MWA_NO_ALT_ENTER)` to prevent unwanted mode switches.
        * Store a reference to the corresponding graphics `CommandQueue` in the `SwapChain` object.
    2. Implement `SwapChain::Present()` to present a buffer to the screen using
       `IDXGISwapChain1::Present(SyncInterval, Flags)`.
        * This method is called once per frame display the buffer with rendered content.
    3. Implement basic `GraphicsContext::ResizeSwapChain(uint32_t Width, uint32_t Height)` to handle window resizing.
        * Flush all work (in the graphics `CommandQueue`) before resizing to ensure no commands are pending on the GPU.
        * Call `IDXGISwapChain1::ResizeBuffers` with the new width and height to adjust the swap chain buffers.

## Renderer (git:dx/04-renderer)

1. Rename the `WindowState` class to `Renderer` to better reflect its purpose.
    * Move the `Renderer` class to the `Graphics` package.
2. Create a RAII wrapper for `ID3D12GraphicsCommandList10`.
    * Prohibit copying to ensure unique ownership.
    * Implement move semantics to transfer ownership over the underlying `ID3D12GraphicsCommandList`.
    * Implement a destructor that closes and submits the command list for execution if both the `CommandQueue` and
      `ID3D12GraphicsCommandList` pointers exist.
3. Implement a private `Renderer::Draw()` method that retrieves a fresh command list via  
   `GraphicsContext::GetCommandList(CommandList<T>& OutCommandList)` to record rendering commands.
    * Refactor `GraphicsContext::Draw()` into `GraphicsContext::GetCommandList(CommandList<T>& OutCommandList)` to
      provide a command list instead of `GraphicsContext::Draw()` call.

## Command Allocator (git:dx/03-command-allocator)

* Add `CommandAllocator` class to manage resources responsible for GPU commands recording for D3D and provide a
  `ID3D12CommandList` object which
  serves the interface for the recording. The `ID3D12CommandList` relies on the `ID3D12CommandAllocator`
  to reset and allocate memory before each use.
    1. Implement `CommandAllocator` to encapsulate both an `ID3D12CommandAllocator` and an
       `ID3D12GraphicsCommandList10`.
        * Prohibit copying to ensure unique ownership and prevent resource conflicts.
    2. Create a factory method `Device::CreateCommandAllocator` that initializes `CommandAllocator` instance.
        * Return a `std::unique_ptr<CommandAllocator>` for the specified command list type and flags.
    3. Add `CommandAllocator::GetID3D12CommandList` method which:
        * Resets the underlying command allocator.
        * Resets the underlying instance of `ID3D12GraphicsCommandList10`.
        * Returns the instance of `ID3D12GraphicsCommandList10` for recording commands.
    4. Integrate with the graphics context to ensure command lists are properly reset and reused each frame.
        * Before recording commands, call `GetID3D12CommandList()` to reset and retrieve the command list.

## Decoupling graphics context updates (git:window/02-decouple-graphics-context-updates)

* Add `WindowState` class to centralize graphics context updates, decoupling immediate window message handling from
  resource updates.
    1. Implement deferred state update pattern: window events set flags/state in App, processed in
       `WindowState::Update()`.
    2. Update `Window::Create(GraphicsContext* GraphicsContext, Window*& OutWindow)` factory method to initialize an
       instance of `WindowState`.
    3. Add `WindowState::OnCreate(HWND hWnd)` and `WindowState::OnResize(int NewWidth, int NewHeight)` to record state
       for next update.
    4. Add `WindowState::Stop()` to signal clean exit; the main application loop calls `WindowState::Update()` until
       exit.
    5. `MainState` manages graphics context lifecycle and main loop control.
    6. Graphics context changes (swap chain creation, resizing) performed once per frame, even if multiple window
       messages received.

## Main Window Pointer (git:window/01-window-message-handler)

* Implement window message handlers to manage window creation and resizing.
    1. Refactor the main window `MainWindow::Create(std::unique_ptr<MainWindow>& OutWindow)` factory method to store a
       pointer to the created window instance in lpParam.
    2. Update the `WindowProc` to handle `WM_NCCREATE`, retrieve the window instance pointer from the `lpParam` data,
       and set it on the user data with `SetWindowLongPtr`.
    3. In the `WindowProc` retrieve the window instance pointer with `GetWindowLongPtr` in subsequent messages and
       invoke the corresponding non-static message handlers.
    4. Handle `WM_CREATE` with the `MainWindow::OnCreate(HWND hWnd)` event handler.
        * Forward the call to `GraphicsContext::OnWindowCreate(HWND hWnd, uint32_t Width, uint32_t Height)` to
          initialize necessary graphics resources, like a swap chain.
    5. Handle `WM_SIZE` with the `MainWindow::OnResize(int Width, int Height)` event handler.
        * Forward the call to `GraphicsContext::OnWindowResize(uint32_t Width, uint32_t Height)` to update graphics
          resources to match the new window size.

## Command Queue (git:dx/02-command-queue)

* Create CommandQueue class that wraps a ID3D12CommandQueue.
    1. Create a factory method `Device::CreateCommandQueue` that initializes and returns `CommandQueue` object.
    2. Implement `CommandQueue::ExecuteCommandList` to execute a given command list.
    3. Implement `CommandQueue::NextFenceValue` to increment and return the next fence value.
    4. Implement `CommandQueue::WaitForFenceValue` a blocking function to wait until the fence reaches a specified
       value.

## Graphics Context and Device (git:dx/01-graphics-context)

* Create GraphicContext class that encaps DX12 device, command queue, etc.
    1. Create DebugLayer class initializing it members with `D3D12GetDebugInterface`, `ID3D12Debug::EnableDebugLayer`
       and `DXGIGetDebugInterface1`. Customize the DebugLayer destructor to report any live objects remaining.
        * Ensure the DebugLayer instance gets destroyed last to report on remaining live objects.
    2. Create Device class that initializes DXGI factory and D3D12 device; and is a factory for other DX12 objects.
        * Update `CMakeLists.txt` to include DX dependencies (`d3d12`, `dxgi`, `dxguid`)
        * Create a factory method `Device::Create` which loops over the available adapters and selects the first one
          that matches the
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
