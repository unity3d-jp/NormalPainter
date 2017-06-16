#include "pch.h"
#include "NormalPainter.h"

#ifdef _WIN32
#ifdef npEnablePenTablet
#define GET_X_LPARAM(lp)                        ((int)(short)LOWORD(lp))
#define GET_Y_LPARAM(lp)                        ((int)(short)HIWORD(lp))

#include "WinTab/MSGPACK.H"
#include "WinTab/WINTAB.H"
#define PACKETDATA	(PK_X | PK_Y | PK_BUTTONS | PK_NORMAL_PRESSURE)
#define PACKETMODE	PK_BUTTONS
#include "WinTab/PKTDEF.H"
#include "WinTab/Import.h"


static HWND g_hwnd;
static LOGCONTEXT	g_logctx = { 0 };
static LRESULT (CALLBACK *UnityWndProc)(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

static HCTX TabletInit(HWND hWnd)
{
    HCTX hctx = NULL;
    UINT wDevice = 0;
    UINT wExtX = 0;
    UINT wExtY = 0;
    UINT wWTInfoRetVal = 0;
    AXIS TabletX = { 0 };
    AXIS TabletY = { 0 };

    // Set option to move system cursor before getting default system context.
    g_logctx.lcOptions |= CXO_SYSTEM;

    // Open default system context so that we can get tablet data
    // in screen coordinates (not tablet coordinates).
    wWTInfoRetVal = _WTInfoA(WTI_DEFSYSCTX, 0, &g_logctx);

    // modify the digitizing region
    wsprintf(g_logctx.lcName, "Normal Painter");

    // We process WT_PACKET (CXO_MESSAGES) messages.
    g_logctx.lcOptions |= CXO_MESSAGES;

    // What data items we want to be included in the tablet packets
    g_logctx.lcPktData = PACKETDATA;

    // Which packet items should show change in value since the last
    // packet (referred to as 'relative' data) and which items
    // should be 'absolute'.
    g_logctx.lcPktMode = PACKETMODE;

    // This bitfield determines whether or not this context will receive
    // a packet when a value for each packet field changes.  This is not
    // supported by the Intuos Wintab.  Your context will always receive
    // packets, even if there has been no change in the data.
    g_logctx.lcMoveMask = PACKETDATA;

    // Which buttons events will be handled by this context.  lcBtnMask
    // is a bitfield with one bit per button.
    g_logctx.lcBtnUpMask = g_logctx.lcBtnDnMask;

    // Set the entire tablet as active
    wWTInfoRetVal = _WTInfoA(WTI_DEVICES + 0, DVC_X, &TabletX);
    wWTInfoRetVal = _WTInfoA(WTI_DEVICES, DVC_Y, &TabletY);

    g_logctx.lcInOrgX = 0;
    g_logctx.lcInOrgY = 0;
    g_logctx.lcInExtX = TabletX.axMax;
    g_logctx.lcInExtY = TabletY.axMax;

    // Guarantee the output coordinate space to be in screen coordinates.  
    g_logctx.lcOutOrgX = GetSystemMetrics(SM_XVIRTUALSCREEN);
    g_logctx.lcOutOrgY = GetSystemMetrics(SM_YVIRTUALSCREEN);
    g_logctx.lcOutExtX = GetSystemMetrics(SM_CXVIRTUALSCREEN); //SM_CXSCREEN );

                                                                  // In Wintab, the tablet origin is lower left.  Move origin to upper left
                                                                  // so that it coincides with screen origin.
    g_logctx.lcOutExtY = -GetSystemMetrics(SM_CYVIRTUALSCREEN);	//SM_CYSCREEN );

                                                                    // Leave the system origin and extents as received:
                                                                    // lcSysOrgX, lcSysOrgY, lcSysExtX, lcSysExtY

                                                                    // open the region
                                                                    // The Wintab spec says we must open the context disabled if we are 
                                                                    // using cursor masks.  
    hctx = _WTOpenA(hWnd, &g_logctx, FALSE);
    return hctx;
}

static LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    static HCTX hCtx = NULL;
    static POINT ptOld, ptNew;
    static UINT prsOld, prsNew;
    static RECT rcClient;
    PACKET pkt;
    BOOL fHandled = TRUE;
    LRESULT lResult = 0L;
    static int xMousePos = 0;
    static int yMousePos = 0;

    switch (message)
    {
    case WM_CREATE:
        hCtx = TabletInit(hWnd);
        if (!hCtx)
        {
            SendMessage(hWnd, WM_DESTROY, 0, 0L);
        }
        break;

    case WT_PACKET:
        if (_WTPacket((HCTX)lParam, (UINT)wParam, &pkt))
        {
            ptOld = ptNew;
            prsOld = prsNew;
            ptNew.x = pkt.pkX;
            ptNew.y = pkt.pkY;
            prsNew = pkt.pkNormalPressure;
        }
        break;

    case WM_ACTIVATE:
        /* if switching in the middle, disable the region */
        if (hCtx)
        {
            _WTEnable(hCtx, GET_WM_ACTIVATE_STATE(wParam, lParam));
            if (hCtx && GET_WM_ACTIVATE_STATE(wParam, lParam))
            {
                _WTOverlap(hCtx, TRUE);
            }
        }
        break;
    }

    if (UnityWndProc) {
        return UnityWndProc(hWnd, message, wParam, lParam);
    }
    else {
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
}

npAPI void npInitializePenTablet()
{
    if (LoadWintab()) {
        g_hwnd = GetForegroundWindow();
        (LONG_PTR&)UnityWndProc = GetWindowLongPtrA(g_hwnd, GWLP_WNDPROC);
        SetWindowLongPtrA(g_hwnd, GWLP_WNDPROC, (LONG_PTR)WndProc);
    }
}

#else // npEnablePenTablet

npAPI void npInitializePenTablet()
{
}

#endif // npEnablePenTablet
#endif // _WIN32
