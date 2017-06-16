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

static HCTX InitTablet(HWND hWnd)
{
    LOGCONTEXT logctx;
    _WTInfoA(WTI_DEFCONTEXT, 0, &logctx);
    logctx.lcOptions |= CXO_SYSTEM | CXO_MESSAGES;
    logctx.lcPktData = PACKETDATA;
    logctx.lcPktMode = PACKETMODE;
    logctx.lcBtnUpMask = logctx.lcBtnDnMask;
    HCTX hctx = _WTOpenA(hWnd, &logctx, TRUE);
    if (hctx) {
        _WTInfoA(WTI_DEVICES, DVC_NPRESSURE, &g_press_axis);
    }
    return hctx;
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


BOOL CALLBACK cbEnumWindows(HWND hwnd, LPARAM lParam)
{
    DWORD procId;
    GetWindowThreadProcessId(hwnd, &procId);
    if (procId != GetCurrentProcessId()) { return TRUE; }

    RECT rect;
    GetWindowRect(hwnd, &rect);

    auto hctx = InitTablet(hwnd);
    if (hctx != nullptr) {
        auto& ctx = g_pen_contexts[hwnd];
        ctx.hctx = hctx;
        (LONG_PTR&)ctx.wndproc = SetWindowLongPtrA(hwnd, GWLP_WNDPROC, (LONG_PTR)npWndProc);
    }

    return TRUE;
}

npAPI void npInitializePenTablet()
{
    static bool s_initialized = false;
    if (s_initialized) { return; }
    s_initialized = true;

    if (LoadWintab()) {
        //cbEnumWindows(GetEditorWindow(), 0);
        EnumWindows(cbEnumWindows, 0);
    }
}

#else // npEnablePenTablet

npAPI void npInitializePenTablet()
{
}

#endif // npEnablePenTablet
#endif // _WIN32
