#include "pch.h"
#include "NormalPainter.h"

#ifdef _WIN32
#ifdef npEnablePenTablet
#define GET_X_LPARAM(lp)                        ((int)(short)LOWORD(lp))
#define GET_Y_LPARAM(lp)                        ((int)(short)HIWORD(lp))

#include "WinTab/MSGPACK.H"
#include "WinTab/WINTAB.H"
#define PACKETDATA (PK_X | PK_Y | PK_BUTTONS | PK_NORMAL_PRESSURE)
#define PACKETMODE PK_BUTTONS
#include "WinTab/PKTDEF.H"
#include "WinTab/Import.h"

#include <dbghelp.h>
#include <psapi.h>
#pragma comment(lib, "dbghelp.lib")


extern float g_pen_pressure;
static AXIS g_press_axis;

struct PenContext
{
    HCTX hctx = nullptr;
    LRESULT(CALLBACK *wndproc)(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) = nullptr;
};

static std::map<HWND, PenContext> g_pen_contexts;


static void* FindSymbolByName(const char *name)
{
    static bool s_first = true;

    if (s_first) {
        s_first = false;

        // set path to main module to symbol search path
        char sympath[MAX_PATH] = "";
        {
            auto ret = ::GetModuleFileNameA(::GetModuleHandleA(nullptr), (LPSTR)sympath, sizeof(sympath));
            for (int i = ret - 1; i > 0; --i) {
                if (sympath[i] == '\\') {
                    sympath[i] = '\0';
                    break;
                }
            }
        }

        DWORD opt = ::SymGetOptions();
        opt |= SYMOPT_DEFERRED_LOADS;
        opt &= ~SYMOPT_UNDNAME;
        ::SymSetOptions(opt);
        ::SymInitialize(::GetCurrentProcess(), sympath, TRUE);
    }

    char buf[sizeof(SYMBOL_INFO) + MAX_SYM_NAME];
    PSYMBOL_INFO sinfo = (PSYMBOL_INFO)buf;
    sinfo->SizeOfStruct = sizeof(SYMBOL_INFO);
    sinfo->MaxNameLen = MAX_SYM_NAME;
    if (::SymFromName(::GetCurrentProcess(), name, sinfo) == FALSE) {
        return nullptr;
    }
    return (void*)sinfo->Address;
}

static HWND GetEditorWindow()
{
    static HWND(*GetMainEditorWindow_)();
    if (!GetMainEditorWindow_) {
        (void*&)GetMainEditorWindow_ = FindSymbolByName("?GetMainEditorWindow@@YAPEAUHWND__@@XZ");
    }
    if (GetMainEditorWindow_) {
        return GetMainEditorWindow_();
    }
    return nullptr;
}

static LRESULT CALLBACK npWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    auto& ctx = g_pen_contexts[hWnd];
    PACKET pkt;

    switch (message)
    {
    case WT_PACKET:
        if (_WTPacket((HCTX)lParam, (UINT)wParam, &pkt)) {
            g_pen_pressure = (float)(pkt.pkNormalPressure - g_press_axis.axMin) / g_press_axis.axMax;
        }
        break;

    case WM_ACTIVATE:
        if (ctx.hctx) {
            _WTEnable(ctx.hctx, GET_WM_ACTIVATE_STATE(wParam, lParam));
            if (ctx.hctx && GET_WM_ACTIVATE_STATE(wParam, lParam)) {
                _WTOverlap(ctx.hctx, TRUE);
            }
        }
        break;
    }

    if (ctx.wndproc) {
        return CallWindowProc(ctx.wndproc, hWnd, message, wParam, lParam);
    }
    else {
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
}

static HCTX InitTablet()
{
    LOGCONTEXT logctx;
    _WTInfoA(WTI_DEFCONTEXT, 0, &logctx);
    logctx.lcOptions |= CXO_SYSTEM | CXO_MESSAGES;
    logctx.lcPktData = PACKETDATA;
    logctx.lcPktMode = PACKETMODE;
    logctx.lcBtnUpMask = logctx.lcBtnDnMask;


    HINSTANCE hInstance = GetModuleHandle(nullptr);

    WNDCLASSEX wc;
    wc.cbSize = sizeof(wc);
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = &npWndProc;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.hInstance = hInstance;
    wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
    wc.lpszMenuName = NULL;
    wc.lpszClassName = "Normal Painter";
    wc.hIconSm = LoadIcon(NULL, IDI_APPLICATION);
    if (::RegisterClassEx(&wc) == NULL) {
        return nullptr;
    }

    HWND hwnd = ::CreateWindowA(
        "Normal Painter", "Normal Painter", WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, NULL, NULL, hInstance, NULL);
    HCTX hctx = _WTOpenA(hwnd, &logctx, TRUE);
    if (hctx) {
        ::SetWindowPos(hwnd, 0, 0, 0, logctx.lcSysExtX, logctx.lcSysExtY, SWP_SHOWWINDOW);

        g_pen_contexts[hwnd].hctx = hctx;
        _WTInfoA(WTI_DEVICES, DVC_NPRESSURE, &g_press_axis);

        new std::thread([hwnd] {
            MSG msg;
            for (;;) {
                while (::PeekMessage(&msg, hwnd, 0, 0, PM_REMOVE)) {
                    ::TranslateMessage(&msg);
                    ::DispatchMessage(&msg);
                }
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
            }
        });
    }
    return hctx;
}

void npInitializePenInput_Win()
{
    static bool s_initialized = false;
    if (s_initialized) { return; }
    s_initialized = true;

    if (LoadWintab()) {
        InitTablet();
    }
}
#endif // npEnablePenTablet
#endif // _WIN32
