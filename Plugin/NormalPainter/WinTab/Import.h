#pragma once

#include <windows.h>
#include "WINTAB.h"

extern UINT (API *_WTInfoA) (UINT, UINT, LPVOID);
extern HCTX (API *_WTOpenA)(HWND, LPLOGCONTEXTA, BOOL);
extern BOOL (API *_WTGetA) (HCTX, LPLOGCONTEXT);
extern BOOL (API *_WTSetA) (HCTX, LPLOGCONTEXT);
extern BOOL (API *_WTClose) (HCTX);
extern BOOL (API *_WTEnable) (HCTX, BOOL);
extern BOOL (API *_WTPacket) (HCTX, UINT, LPVOID);
extern BOOL (API *_WTOverlap) (HCTX, BOOL);
extern BOOL (API *_WTSave) (HCTX, LPVOID);
extern BOOL (API *_WTConfig) (HCTX, HWND);
extern HCTX (API *_WTRestore) (HWND, LPVOID, BOOL);
extern BOOL (API *_WTExtSet) (HCTX, UINT, LPVOID);
extern BOOL (API *_WTExtGet) (HCTX, UINT, LPVOID);
extern BOOL (API *_WTQueueSizeSet) (HCTX, int);
extern int  (API *_WTDataPeek) (HCTX, UINT, UINT, int, LPVOID, LPINT);
extern int  (API *_WTPacketsGet) (HCTX, int, LPVOID);
extern HMGR (API *_WTMgrOpen) (HWND, UINT);
extern BOOL (API *_WTMgrClose) (HMGR);
extern HCTX (API *_WTMgrDefContext) (HMGR, BOOL);
extern HCTX (API *_WTMgrDefContextEx) ( HMGR, UINT, BOOL );


BOOL LoadWintab( void );
void UnloadWintab( void );
