//#include "client.h"
//#include "gpu.h"
//#include "../log.h"
//#include "../cvar.h"
//
//#include <Windows.h>
//#include <assert.h>
//
//#define WINDOW_CLASS_NAME "battlecry"
//#define ICON_NAME "BATTLECRY_ICON"
//
//static cvar_t cv_window_width = {
//    .name = "window_width",
//    .description = "Width of the window",
//    .type = CVAR_TYPE_INT,
//    .flags = CVAR_SAVE,
//    .int_value = 1280
//};
//
//static cvar_t cv_window_height = {
//    .name = "window_width",
//    .description = "Width of the window",
//    .type = CVAR_TYPE_INT,
//    .flags = CVAR_SAVE,
//    .int_value = 1280
//};
//
//static HWND handle;
//static bool fullscreen;
//static bool borderless;
//static bool minimized;
//static bool maximized;
//static RECT windowed_rect;
//static RECT fullscreen_rect;
//static RECT client_rect;
//
//static const DWORD window_style = WS_OVERLAPPED | WS_MINIMIZEBOX | WS_MAXIMIZEBOX;
//
//static void set_fullscreen(bool fullscreen)
//{
//    assert(window);
//
//    if (fullscreen)
//    {
//        if (!window->fullscreen)
//        {
//            // Save the old window rect so we can restore it when exiting fullscreen mode.
//            GetWindowRect(window->handle, &window->windowed_rect);
//        }
//
//        // Make the window borderless so that the client area can fill the screen.
//        SetWindowLong(window->handle, GWL_STYLE, WS_SYSMENU | WS_POPUP | WS_CLIPCHILDREN | WS_CLIPSIBLINGS | WS_VISIBLE);
//
//        // Get the settings of the durrent display index. We want the app to go into
//        // fullscreen mode on the display that supports Independent Flip.
//        HMONITOR current_monitor = MonitorFromWindow(window->handle, MONITOR_DEFAULTTOPRIMARY);
//        MONITORINFOEX monitor_info = {
//            .cbSize = sizeof(MONITORINFOEX)
//        };
//
//#pragma clang diagnostic push
//#pragma clang diagnostic ignored "-Wincompatible-pointer-types"
//        bool infoRead = GetMonitorInfo(current_monitor, &monitor_info);
//#pragma clang diagnostic pop
//
//        pApp->mSettings.mWindowX = monitor_info.rcMonitor.left;
//        pApp->mSettings.mWindowY = monitor_info.rcMonitor.top;
//
//        SetWindowPos(
//            window->handle, HWND_NOTOPMOST, monitor_info.rcMonitor.left, monitor_info.rcMonitor.top, monitor_info.rcMonitor.right - monitor_info.rcMonitor.left,
//            monitor_info.rcMonitor.bottom - monitor_info.rcMonitor.top, SWP_FRAMECHANGED | SWP_NOACTIVATE);
//
//        ShowWindow(window->handle, SW_MAXIMIZE);
//
//        onResize(winDesc, monitor_info.rcMonitor.right - monitor_info.rcMonitor.left,
//                 monitor_info.rcMonitor.bottom - monitor_info.rcMonitor.top);
//    }
//    else
//    {
//        SetWindowLong(window->handle, GWL_STYLE, window_style);
//
//        SetWindowPos(
//            window->handle,
//            HWND_NOTOPMOST,
//            window->windowed_rect.left,
//            window->windowed_rect.top,
//            window->windowed_rect.right - window->windowed_rect.left,
//            window->windowed_rect.bottom - window->windowed_rect.top,
//            SWP_FRAMECHANGED | SWP_NOACTIVATE | SWP_NOOWNERZORDER);
//
//        if (window->maximized)
//        {
//            ShowWindow(window->handle, SW_MAXIMIZE);
//        }
//        else
//        {
//            ShowWindow(window->handle, SW_NORMAL);
//        }
//    }
//}
//
//LRESULT CALLBACK win_proc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
//{
//    switch (message)
//    {
//        case WM_NCPAINT:
//        case WM_WINDOWPOSCHANGED:
//        case WM_STYLECHANGED:
//        {
//            return DefWindowProcW(hwnd, message, wParam, lParam);
//        }
//        case WM_DISPLAYCHANGE:
//        {
//            adjustWindow(gWindow);
//            break;
//        }
//        case WM_GETMINMAXINFO:
//        {
//            LPMINMAXINFO lpMMI = (LPMINMAXINFO)lParam;
//
//            // These sizes should be well enough to accommodate any possible
//            // window styles in full screen such that the client rectangle would
//            // be equal to the fullscreen rectangle without cropping or movement.
//            // These sizes were tested with 450% zoom, and should support up to
//            // about 550% zoom, if such option exists.
//            if (!gWindow->fullScreen)
//            {
//                LONG zoomOffset = 128;
//                lpMMI->ptMaxPosition.x = -zoomOffset;
//                lpMMI->ptMaxPosition.y = -zoomOffset;
//                lpMMI->ptMinTrackSize.x = zoomOffset;
//                lpMMI->ptMinTrackSize.y = zoomOffset;
//                lpMMI->ptMaxTrackSize.x = gWindow->clientRect.left + getRectWidth(gWindow->clientRect) + zoomOffset;
//                lpMMI->ptMaxTrackSize.y = gWindow->clientRect.top + getRectHeight(gWindow->clientRect) + zoomOffset;
//            }
//            break;
//        }
//        case WM_ERASEBKGND:
//        {
//            // Make sure to keep consistent background color when resizing.
//            HDC hdc = (HDC)wParam;
//            RECT rc;
//            HBRUSH hbrWhite = CreateSolidBrush(0x00000000);
//            GetClientRect(hwnd, &rc);
//            FillRect(hdc, &rc, hbrWhite);
//            break;
//        }
//        case WM_WINDOWPOSCHANGING:
//        case WM_MOVE:
//        {
//            UpdateWindowDescFullScreenRect(gWindow);
//            if (!gWindow->fullScreen)
//                UpdateWindowDescWindowedRect(gWindow);
//            break;
//        }
//        case WM_STYLECHANGING:
//        {
//            break;
//        }
//        case WM_SIZE:
//        {
//            switch (wParam)
//            {
//                case SIZE_RESTORED:
//                case SIZE_MAXIMIZED:
//                    gWindow->minimized = false;
//                    if (!gWindow->fullScreen && !gWindowIsResizing)
//                    {
//                        UpdateWindowDescClientRect(gWindow);
//                        onResize(gWindow, getRectWidth(gWindow->clientRect), getRectHeight(gWindow->clientRect));
//                    }
//                    break;
//                case SIZE_MINIMIZED:
//                    gWindow->minimized = true;
//                    break;
//                default:
//                    break;
//            }
//
//            break;
//        }
//        case WM_ENTERSIZEMOVE:
//        {
//            gWindowIsResizing = true;
//            break;
//        }
//        case WM_EXITSIZEMOVE:
//        {
//            gWindowIsResizing = false;
//            if (!gWindow->fullScreen)
//            {
//                UpdateWindowDescClientRect(gWindow);
//                onResize(gWindow, getRectWidth(gWindow->clientRect), getRectHeight(gWindow->clientRect));
//            }
//            break;
//        }
//        case WM_SETCURSOR:
//        {
//            if (LOWORD(lParam) == HTCLIENT)
//            {
//                if (!gCursorInsideRectangle)
//                {
//                    HCURSOR cursor = LoadCursor(NULL, IDC_ARROW);
//                    SetCursor(cursor);
//
//                    gCursorInsideRectangle = true;
//                }
//            }
//            else
//            {
//                gCursorInsideRectangle = false;
//                return DefWindowProcW(hwnd, message, wParam, lParam);
//            }
//            break;
//        }
//        case WM_DESTROY:
//        case WM_CLOSE:
//        {
//            PostQuitMessage(0);
//            break;
//        }
//        default:
//        {
//            return DefWindowProcW(hwnd, message, wParam, lParam);
//        }
//    }
//    return 0;
//}
//
//void client_init()
//{
//    assert(window);
//
//    // Window class
//    HINSTANCE instance = (HINSTANCE)GetModuleHandle(NULL);
//    WNDCLASS window_class = {
//        .lpfnWndProc = win_proc,
//        .hInstance = instance,
//        .hIcon = LoadImage(instance, ICON_NAME, IMAGE_ICON, 0, 0, LR_DEFAULTSIZE | LR_SHARED),
//        .hCursor = LoadCursor(NULL, IDC_ARROW),
//        .lpszClassName = WINDOW_CLASS_NAME
//    };
//    bool success = RegisterClass(&window_class) != 0;
//    assert(success);
//
//    // Monitors
//    DISPLAY_DEVICE display_device = {
//        .cb = sizeof(DISPLAY_DEVICE)
//    };
//
//    u32 monitor_count;
//    for (i32 display_device_index = 0;; display_device_index++)
//    {
//        if (!EnumDisplayDevices(NULL, display_device_index, &display_device, 0))
//            break;
//
//        if ((display_device.StateFlags & DISPLAY_DEVICE_ACTIVE) == 0)
//            continue;
//
//        for (i32 display_index =0;; display_index++)
//        {
//
//        }
//    }
//}
//
//void client_quit()
//{
//
//}