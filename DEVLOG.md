# Changelog

## [#18 Upload Vertex Data](https://github.com/dimatcimbal/DXStarter/pull/18)

Added both Vertex and Upload buffers; added vertex data upload functionality.

1. **Resource Management Architecture**:
   * Created a base `Resource` class to wrap `ID3D12Resource2`.
   * Added `DefaultBuffer` class inheriting from `Resource` for GPU-accessible buffers with heap type and size tracking.
   * Added `UploadBuffer` class inheriting from `DefaultBuffer` for CPU-to-GPU data transfer with RAII mapping via `BufferRange` class.
   * Added `ByteUtil` utility class with `AlignTo256Bytes` method for D3D12 constant buffer alignment requirements.

2. **Vertex Data Loading System**:
   * Added `Renderer::LoadVertexData` method for uploading vertex data using the upload buffer onto GPU.
   * Added `CommandList10::CopyBufferRegion` method for efficient buffer-to-buffer copying.
   * Updated `Main.cpp` to create vertex data for a simple triangle and upload it onto the GPU.

3. **Command List Architecture Refactoring**:
   * Separated general purpose `CommandList10` from frame-specific `FrameCommandList10` which holds a reference to the swap chain.
   * Made `CommandList10::TransitionResource` a template method accepting any `Resource`-derived type.
   * Added `CommandList10::operator->()` for direct access methods of the underlying `ID3D12GraphicsCommandList10`.

4. **Device Buffer Creation**:
   * Added `Device::CreateBuffer` template method for creating any `DefaultBuffer`-derived buffer type.
   * Updated `Device::GetCommandList` and added `Device::GetFrameCommandList` for different command list usage patterns.

## [#13 Code Cleanup](https://github.com/dimatcimbal/DXStarter/pull/13)

Refactored the graphics architecture by removing the `GraphicsContext` class and consolidating its functionality into
the `Device` class for better separation of concerns and simplified resource management.

1. Deleted `GraphicsContext.cpp` and `GraphicsContext.h` files, eliminating the intermediate layer between `Device`
   and `Renderer`. Moved all graphics resource creation and management directly into the `Device` class:
    * Added static factory methods `Device::CreateCommandAllocator`, `Device::CreateCommandQueue`,
      `Device::CreateDescriptorHeap`, and `Device::GetBestDevice` to handle resource creation and adapter selection.
    * Updated `Device::Create` to initialize all core graphics resources (DebugLayer, CommandQueue,
      CommandAllocator, RTV DescriptorHeap) during device creation.
    * Added `Device::GetCommandList` method to provide command lists directly to the renderer.
2. Updated `Device::CreateSwapChain` to use default parameters and removed the need for external CommandQueue
   parameter, using hardcoded constants and accessing the graphics command queue through the device's internal
   `mGraphicsQueue` member.
3. Refactored `Renderer` to work directly with `Device` instead of `GraphicsContext`:
    * Changed constructor to accept `Device*` instead of `GraphicsContext*`.
    * Added `mSwapChain` as an owned resource in `Renderer`.
    * Updated `Renderer::Draw()` to call `Device::GetCommandList()` instead of `GraphicsContext::GetCommandList()`.
    * Moved swap chain creation and resizing logic from `GraphicsContext` into `Renderer::Update()`.
    * Added proper window dimension caching with `mNewWidth`/`mNewHeight` for resize operations.
4. Added proper destructor to `SwapChain` class that calls `FlushAll()` to ensure
   GPU operations complete before destruction. Updated `Main.cpp` to replace `GraphicsContext::Create()` with direct
   `Device::Create()` call. Relocated `GRAPHICS_FEATURE_LEVEL` and `SWAP_CHAIN_BUFFER_COUNT` constants from
   `GraphicsContext.h` to `Device.h` where they are used.

## [#12 Back Buffers](https://github.com/dimatcimbal/DXStarter/pull/12)

Extended the `SwapChain` class to manage back buffer resources using a new `ColorBuffer` class that wraps both the
resource and its RTV handle.

1. Added `SwapChain::mBackBuffers` as a vector of `std::unique_ptr<ColorBuffer>` to hold references to the back
   buffer resources and their RTVs.
2. Added `SwapChain::mCurrentBackBufferIndex` to track the current back buffer index.
3. Updated `SwapChain::FlushAll` to release references by setting vector elements to `nullptr`, keeping the vector
   size intact.
4. Refactored `SwapChain::BuffersReadTo` to fill the vector with `ColorBuffer` objects, creating RTVs for each
   buffer using `Device::CreateRTV`.
5. Added `SwapChain::BeginFrame` and `SwapChain::EndFrame` methods to handle resource state transitions for the back
   buffer at the start and end of each frame:
    * `SwapChain::BeginFrame`:
        - Transitioned the current back buffer to `D3D12_RESOURCE_STATE_RENDER_TARGET` using
          `CommandList10::TransitionResource`.
        - Cleared the render target with `CommandList10::ClearTarget`.
        - Set the render target with `CommandList10::SetRenderTarget`.
    * `SwapChain::EndFrame`:
        - Transitioned the current back buffer to `D3D12_RESOURCE_STATE_PRESENT` using
          `CommandList10::TransitionResource`.
6. Implemented `SwapChain::GetCurrentBackBuffer` to return a pointer to the current `ColorBuffer`.
7. Removed `SwapChain::GetCurrentBackBufferView` (now handled by `ColorBuffer::GetRTV`).
8. Descriptor heap creation and RTV allocation are now handled in `Device::Create` and `Device::CreateRTV`, not in
   GraphicsContext.
9. GraphicsContext no longer owns the RTV heap; it is managed by Device.

## [#11 Descriptor Heaps](https://github.com/dimatcimbal/DXStarter/pull/11)

Added a new `DescriptorHeap` class that wraps the DirectX 12 descriptor heap `ID3D12DescriptorHeap`, keeping track of
its type, size, count.

1. Added a `CreateHandles` method that allocates descriptor handles from the heap, automatically tracks usage via
   both `DescriptorHeap::mCurrentHandle` value and `DescriptorHeap::mFreeDescriptorCount` counter, and reports
   errors if running out of descriptors occurs.
2. Updated `Device::Create` method to create rtvHeap object of type `DescriptorHeap`.
3. Verified that descriptor heap allocation works by testing handle creation during the device startup.

## [#9 Swap Chain](https://github.com/dimatcimbal/DXStarter/pull/9)

Created SwapChain class that wraps a `IDXGISwapChain1`.

1. Created a factory method `Device::CreateSwapChain` that initializes and returns a `SwapChain` object.
   * Described the swap chain using a `DXGI_SWAP_CHAIN_DESC1` struct, specifying buffer count, format, usage, and
   swap effect.
   * Used `IDXGIFactory7::CreateSwapChainForHwnd` to create the swap chain for a window, passing the command queue,
   window handle, and swap chain description.
   * Disabled the Alt+Enter fullscreen toggle feature with
   `IDXGIFactory4::MakeWindowAssociation(HWND, DXGI_MWA_NO_ALT_ENTER)` to prevent unwanted mode switches.
   * Stored a reference to the corresponding graphics `CommandQueue` in the `SwapChain` object.
2. Implemented `SwapChain::Present()` to present a buffer to the screen using
   `IDXGISwapChain1::Present(SyncInterval, Flags)`.
    * This method is called once per frame to display the buffer with rendered content.
3. Implemented basic `GraphicsContext::ResizeSwapChain(uint32_t Width, uint32_t Height)` to handle window resizing.
    * Flushed all work (in the graphics `CommandQueue`) before resizing to ensure no commands are pending on the
      GPU.
    * Called `IDXGISwapChain1::ResizeBuffers` with the new width and height to adjust the swap chain buffers.

## [#7 Renderer](https://github.com/dimatcimbal/DXStarter/pull/7)

Renamed the `WindowState` class to `Renderer` to better reflect its purpose.

1. Moved the `Renderer` class to the `Graphics` package.
2. Created a RAII wrapper for `ID3D12GraphicsCommandList10`.
    * Prohibited copying to ensure unique ownership.
    * Implemented move semantics to transfer ownership over the underlying `ID3D12GraphicsCommandList`.
    * Implemented a destructor that closes and submits the command list for execution if both the `CommandQueue` and
      `ID3D12GraphicsCommandList` pointers exist.
3. Implemented a private `Renderer::Draw()` method that retrieves a fresh command list via
   `GraphicsContext::GetCommandList(CommandList<T>& OutCommandList)` to record rendering commands.
    * Refactored `GraphicsContext::Draw()` into `GraphicsContext::GetCommandList(CommandList<T>& OutCommandList)` to
      provide a command list instead of `GraphicsContext::Draw()` call.

## [#6 Command Allocator](https://github.com/dimatcimbal/DXStarter/pull/6)

Added `CommandAllocator` class to manage resources responsible for GPU commands recording for D3D and provide a
`ID3D12CommandList` object which serves the interface for the recording. The `ID3D12CommandList` relies on the
`ID3D12CommandAllocator` to reset and allocate memory before each use.

1. Implemented `CommandAllocator` to encapsulate both an `ID3D12CommandAllocator` and an
   `ID3D12GraphicsCommandList10`.
   * Prohibited copying to ensure unique ownership and prevent resource conflicts.
2. Created a factory method `Device::CreateCommandAllocator` that initializes `CommandAllocator` instance.
    * Returned a `std::unique_ptr<CommandAllocator>` for the specified command list type and flags.
3. Added `CommandAllocator::GetID3D12CommandList` method which:
    * Resets the underlying command allocator.
    * Resets the underlying instance of `ID3D12GraphicsCommandList10`.
    * Returns the instance of `ID3D12GraphicsCommandList10` for recording commands.
4. Integrated with the graphics context to ensure command lists are properly reset and reused each frame.
    * Before recording commands, call `GetID3D12CommandList()` to reset and retrieve the command list.

## [#5 Decoupling graphics context updates](https://github.com/dimatcimbal/DXStarter/pull/5)

Added `WindowState` class to centralize graphics context updates, decoupling immediate window message handling from
resource updates.

1. Implemented deferred state update pattern: window events set flags/state in App, processed in
   `WindowState::Update()`.
2. Updated `Window::Create(GraphicsContext* GraphicsContext, Window*& OutWindow)` factory method to initialize an
   instance of `WindowState`.
3. Added `WindowState::OnCreate(HWND hWnd)` and `WindowState::OnResize(int NewWidth, int NewHeight)` to record state
   for next update.
4. Added `WindowState::Stop()` to signal clean exit; the main application loop calls `WindowState::Update()` until
   exit.
5. `MainState` manages graphics context lifecycle and main loop control.
6. Graphics context changes (swap chain creation, resizing) performed once per frame, even if multiple window
   messages received.

## [#4 Storing a pointer to the MainWindow](https://github.com/dimatcimbal/DXStarter/pull/4)

Implemented window message handlers to manage window creation and resizing.

1. Refactored the main window `MainWindow::Create(std::unique_ptr<MainWindow>& OutWindow)` factory method to store a
   pointer to the created window instance in lpParam.
2. Updated the `WindowProc` to handle `WM_NCCREATE`, retrieve the window instance pointer from the `lpParam` data,
   and set it on the user data with `SetWindowLongPtr`.
3. In the `WindowProc` retrieve the window instance pointer with `GetWindowLongPtr` in subsequent messages and
   invoke the corresponding non-static message handlers.
4. Handled `WM_CREATE` with the `MainWindow::OnCreate(HWND hWnd)` event handler.
    * Forwarded the call to `GraphicsContext::OnWindowCreate(HWND hWnd, uint32_t Width, uint32_t Height)` to
      initialize necessary graphics resources, like a swap chain.
5. Handled `WM_SIZE` with the `MainWindow::OnResize(int Width, int Height)` event handler.
    * Forwarded the call to `GraphicsContext::OnWindowResize(uint32_t Width, uint32_t Height)` to update graphics
      resources to match the new window size.

## [#3 Command Queue](https://github.com/dimatcimbal/DXStarter/pull/3)

Created CommandQueue class that wraps a ID3D12CommandQueue.

1. Created a factory method `Device::CreateCommandQueue` that initializes and returns `CommandQueue` object.
2. Implemented `CommandQueue::ExecuteCommandList` to execute a given command list.
3. Implemented `CommandQueue::NextFenceValue` to increment and return the next fence value.
4. Implemented `CommandQueue::WaitForFenceValue` a blocking function to wait until the fence reaches a specified
   value.

## [#2 Graphics Context and Device](https://github.com/dimatcimbal/DXStarter/pull/2)

Created GraphicsContext class that encapsulates DX12 device, command queue, etc.

1. Created DebugLayer class initializing it members with `D3D12GetDebugInterface`, `ID3D12Debug::EnableDebugLayer`
   and `DXGIGetDebugInterface1`. Customized the DebugLayer destructor to report any live objects remaining.
   * Ensured the DebugLayer instance gets destroyed last to report on remaining live objects.
2. Created Device class that initializes DXGI factory and D3D12 device; and is a factory for other DX12 objects.
    * Updated `CMakeLists.txt` to include DX dependencies (`d3d12`, `dxgi`, `dxguid`)
    * Created a factory method `Device::Create` which loops over the available adapters and selects the first one
      that matches the
      criteria (is hardware, supports the requested feature level, has the highest video memory).

## [#1 Main Window](https://github.com/dimatcimbal/DXStarter/pull/1)

Created an application window with the following steps:

1. Implemented the standard `WinMain(HINSTANCE, HINSTANCE, LPSTR, int)` handler.
   * `WinMain` is the entry point for Windows applications, similar to `main` in console applications.
   * Created the main window instance and enter the main application loop.
2. Used `GetModuleHandle(NULL)` to retrieve the handle to the current module.
3. Filled out a `WNDCLASSEX` structure to define the window's properties.
    * Include a pointer to the window procedure (e.g., `DefWindowProc` or a custom implementation).
    * Specify the class name, cursor, and other attributes.
4. Used `RegisterClassEx` to register the class with the system.
    * Check the return value to ensure the registration succeeded.
5. Called `CreateWindowEx` with the registered class name and desired styles.
    * Handle errors if the function returns `NULL`.
6. Handled rendering and processing of window messages:
    * Used `GetMessage` to retrieve messages from the message queue.
        * Check the return value to detect when the application should exit.
    * Used `TranslateMessage` to process keyboard input.
    * Used `DispatchMessage` to send messages to the window procedure.
    * When the loop exits, use `msg.wParam` as the application's exit code.
7. Defined `NOMINMAX` and `UNICODE` in `CMakeLists.txt` with `target_compile_definitions` to exclude min and max
   macros and use wide-character versions of WinAPI functions in macros like OutputDebugString.
