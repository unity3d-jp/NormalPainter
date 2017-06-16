#include "pch.h"
#include "Import.h"


static HINSTANCE g_hwintab;

UINT (API *_WTInfoA) (UINT, UINT, LPVOID);
HCTX (API *_WTOpenA)(HWND, LPLOGCONTEXTA, BOOL);
BOOL (API *_WTGetA) (HCTX, LPLOGCONTEXT);
BOOL (API *_WTSetA) (HCTX, LPLOGCONTEXT);
BOOL (API *_WTClose) (HCTX);
BOOL (API *_WTEnable) (HCTX, BOOL);
BOOL (API *_WTPacket) (HCTX, UINT, LPVOID);
BOOL (API *_WTOverlap) (HCTX, BOOL);
BOOL (API *_WTSave) (HCTX, LPVOID);
BOOL (API *_WTConfig) (HCTX, HWND);
HCTX (API *_WTRestore) (HWND, LPVOID, BOOL);
BOOL (API *_WTExtSet) (HCTX, UINT, LPVOID);
BOOL (API *_WTExtGet) (HCTX, UINT, LPVOID);
BOOL (API *_WTQueueSizeSet) (HCTX, int);
int  (API *_WTDataPeek) (HCTX, UINT, UINT, int, LPVOID, LPINT);
int  (API *_WTPacketsGet) (HCTX, int, LPVOID);
HMGR (API *_WTMgrOpen) (HWND, UINT);
BOOL (API *_WTMgrClose) (HMGR);
HCTX (API *_WTMgrDefContext) (HMGR, BOOL);
HCTX (API *_WTMgrDefContextEx) ( HMGR, UINT, BOOL );



BOOL LoadWintab( void )
{
    g_hwintab = ::LoadLibraryA( "Wintab32.dll" );
    if (!g_hwintab) { return FALSE; }

#define IMPORT(Func) \
    (void*&)_##Func = ::GetProcAddress(g_hwintab, #Func); if (!_##Func) { UnloadWintab(); return FALSE; }

    IMPORT(WTOpenA);
    IMPORT(WTInfoA);
    IMPORT(WTGetA);
    IMPORT(WTSetA);
    IMPORT(WTPacket);
    IMPORT(WTClose);
    IMPORT(WTEnable);
    IMPORT(WTOverlap);
    IMPORT(WTSave);
    IMPORT(WTConfig);
    IMPORT(WTRestore);
    IMPORT(WTExtSet);
    IMPORT(WTExtGet);
    IMPORT(WTQueueSizeSet);
    IMPORT(WTDataPeek);
    IMPORT(WTPacketsGet);
    IMPORT(WTMgrOpen);
    IMPORT(WTMgrClose);
    IMPORT(WTMgrDefContext);
    IMPORT(WTMgrDefContextEx);

#undef IMPORT

    return TRUE;
}


void UnloadWintab( void )
{
    if (g_hwintab)
    {
        ::FreeLibrary(g_hwintab);
        g_hwintab = nullptr;
    }

    _WTOpenA    = nullptr;
    _WTClose    = nullptr;
    _WTInfoA    = nullptr;
    _WTPacket   = nullptr;
    _WTEnable   = nullptr;
    _WTOverlap  = nullptr;
    _WTSave     = nullptr;
    _WTConfig   = nullptr;
    _WTGetA     = nullptr;
    _WTSetA     = nullptr;
    _WTRestore  = nullptr;
    _WTExtSet   = nullptr;
    _WTExtGet   = nullptr;
    _WTQueueSizeSet = nullptr;
    _WTDataPeek     = nullptr;
    _WTPacketsGet   = nullptr;
    _WTMgrOpen      = nullptr;
    _WTMgrClose     = nullptr;
    _WTMgrDefContext    = nullptr;
    _WTMgrDefContextEx  = nullptr;
}
