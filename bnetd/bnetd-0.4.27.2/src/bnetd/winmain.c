/*
 * Copyright (C) 2001  Erik Latoshek [forester] (laterk@inbox.lv)
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */
#include "winmain.h"
#include "common/setup_before.h"
#include "common/eventlog.h"
#include "common/version.h"
#include "server.h"

#include <windows.h>
#include <windowsx.h>
#include <process.h>
#include <stdio.h>
#include <richedit.h>
#include <commctrl.h>

#include "resource.h"

#define WM_SHELLNOTIFY          (WM_USER+1)
#define CONSOLE_MAX_SIZE	0xFFFF

int extern main(int, char*[]);

void static         guiThread(void*);
void static         guiAddText(const char *, COLORREF, int);
void static         guiDEAD(char*);
void static         guiMoveWindow(HWND, RECT*);
void static         guiClearLogWindow(void);
void static         guiKillTrayIcon(void);

LRESULT static CALLBACK guiWndProc(HWND, UINT, WPARAM, LPARAM);
void static         guiOnCommand(HWND, int, HWND, UINT);
void static         guiOnMenuSelect(HWND, HMENU, int, HMENU, UINT);
int  static         guiOnShellNotify(int, int);
BOOL static         guiOnCreate(HWND, LPCREATESTRUCT);
void static         guiOnClose(HWND);
void static         guiOnSize(HWND, UINT, int, int);
void static         guiOnPaint(HWND);
void static         guiOnCaptureChanged(HWND);
BOOL static         guiOnSetCursor(HWND, HWND, UINT, UINT);
void static         guiOnMouseMove(HWND, int, int, UINT);
void static         guiOnLButtonDown(HWND, BOOL, int, int, UINT);
void static         guiOnLButtonUp(HWND, int, int, UINT);
void static         guiInsertColumn(HWND, int, char*, int);

enum mode { mode_vdivide=1, mode_scroll=2 };

struct gui_struc{
    HWND    hwnd;
    HMENU   hmenuTray;
    HWND    hwndConsole;
    HWND    hwndUsers;
    HWND    hwndTree;
    HWND    hwndStatus;
    int     y_ratio;
    int     x_ratio;
    HANDLE  event_ready;
    BOOL    main_finished;
    int     mode;
    int     console_size;
    char    szDefaultStatus[128];

    RECT    rectHDivider,
            rectVDivider,
            rectConsole,
            rectUsers,
            rectConsoleEdge,
            rectUsersEdge;

    WPARAM  wParam;
    LPARAM  lParam;
};
static struct gui_struc gui;


int CALLBACK WinMain(HINSTANCE hInstance, HINSTANCE reserved,
 LPSTR lpCmdLine, int nCmdShow){
 int result;

    gui.main_finished = FALSE;
    gui.event_ready = CreateEvent(NULL, FALSE, FALSE, NULL);
    _beginthread(guiThread, 0, (void*)hInstance);
    WaitForSingleObject(gui.event_ready, INFINITE);

    result = main(__argc, __argv);
    gui.main_finished = TRUE;
    eventlog(eventlog_level_info,"WinMain","server exited ( return : %i )", result);
    WaitForSingleObject(gui.event_ready, INFINITE);

    return 0;
}


void static guiThread(void *param){
 WNDCLASSEX wc;
 MSG msg;
 HACCEL haccel;

    InitCommonControls();
    LoadLibrary("RichEd20.dll");

    wc.cbSize = sizeof(WNDCLASSEX);
    wc.style = CS_HREDRAW|CS_VREDRAW;
    wc.lpfnWndProc = guiWndProc;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.hInstance = (HINSTANCE)param;
    wc.hIcon = LoadIcon(wc.hInstance, MAKEINTRESOURCE(IDR_ICON32X32));
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = 0;
    wc.lpszMenuName = MAKEINTRESOURCE(IDR_MENU);
    wc.lpszClassName = "BnetdWndClass";
    wc.hIconSm = NULL;
    if(!RegisterClassEx( &wc )) guiDEAD("cant register WNDCLASS");

    gui.hwnd = CreateWindowEx(
     0,
     wc.lpszClassName,
     "BnetD "BNETD_VERSION,
     WS_OVERLAPPEDWINDOW,
     CW_USEDEFAULT,
     CW_USEDEFAULT,
     CW_USEDEFAULT,
     CW_USEDEFAULT,
     NULL,
     NULL,
     (HINSTANCE)param,
     NULL);
    if(!gui.hwnd) guiDEAD("cant create window");

    ShowWindow(gui.hwnd, SW_SHOW);
    PulseEvent(gui.event_ready);

    haccel = LoadAccelerators((HINSTANCE)param, MAKEINTRESOURCE(IDA_ACCEL));

    while( GetMessage( &msg, NULL, 0, 0 ) ){
        if(!TranslateAccelerator(gui.hwnd, haccel, &msg)){
            TranslateMessage( &msg );
            DispatchMessage( &msg );
        }
    }

}


LRESULT static CALLBACK guiWndProc(
 HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam){

    gui.wParam = wParam;
    gui.lParam = lParam;
    switch(message){
        HANDLE_MSG(hwnd, WM_CREATE, guiOnCreate);
        HANDLE_MSG(hwnd, WM_COMMAND, guiOnCommand);
        HANDLE_MSG(hwnd, WM_MENUSELECT, guiOnMenuSelect);
        HANDLE_MSG(hwnd, WM_SIZE, guiOnSize);
        HANDLE_MSG(hwnd, WM_CLOSE, guiOnClose);
        HANDLE_MSG(hwnd, WM_PAINT, guiOnPaint);
        HANDLE_MSG(hwnd, WM_SETCURSOR, guiOnSetCursor);
        HANDLE_MSG(hwnd, WM_LBUTTONDOWN, guiOnLButtonDown);
        HANDLE_MSG(hwnd, WM_LBUTTONUP, guiOnLButtonUp);
        HANDLE_MSG(hwnd, WM_MOUSEMOVE, guiOnMouseMove);
        case WM_CAPTURECHANGED:
            guiOnCaptureChanged((HWND)lParam);
            return 0;
        case WM_SHELLNOTIFY:
            return guiOnShellNotify(wParam, lParam);
    }
    return DefWindowProc(hwnd, message, wParam, lParam);
}


BOOL static guiOnCreate(HWND hwnd, LPCREATESTRUCT lpCreateStruct){

    gui.hwndConsole = CreateWindowEx(
     0,
     RICHEDIT_CLASS,
     NULL,
     WS_CHILD|WS_VISIBLE|ES_READONLY|ES_MULTILINE|WS_VSCROLL|WS_HSCROLL,
     0, 0,
     0, 0,
     hwnd,
     0,
     GetWindowInstance(hwnd),
     NULL);
    if(!gui.hwndConsole) return FALSE;
    SendMessage(gui.hwndConsole, EM_LIMITTEXT, 0xFFFF, 0);

    gui.hwndUsers = CreateWindowEx(
     0,
     WC_LISTVIEW,
     NULL,
     WS_CHILD|WS_VISIBLE|LVS_REPORT,
     0, 0,
     0, 0,
     hwnd,
     0,
     GetWindowInstance(hwnd),
     NULL);
    if(!gui.hwndUsers) return FALSE;

    guiInsertColumn(gui.hwndUsers, 0, "Session Id", 70);
    guiInsertColumn(gui.hwndUsers, 1, "Class:State", 100);
    guiInsertColumn(gui.hwndUsers, 2, "Tag:Version", 70);
    guiInsertColumn(gui.hwndUsers, 3, "User Name", 100);
    guiInsertColumn(gui.hwndUsers, 4, "IP", 80);
    guiInsertColumn(gui.hwndUsers, 5, "Latency", 50);

    strcpy( gui.szDefaultStatus, "Void" );
    gui.hwndStatus = CreateStatusWindow(
     WS_CHILD | WS_VISIBLE,
     gui.szDefaultStatus,
     hwnd,
     0);
    if( !gui.hwndStatus ) return FALSE;

    gui.hmenuTray = CreatePopupMenu();
    AppendMenu(gui.hmenuTray, MF_STRING, IDM_RESTORE, "&Restore");
    AppendMenu(gui.hmenuTray, MF_SEPARATOR, 0, 0);
    AppendMenu(gui.hmenuTray, MF_STRING, IDM_EXIT, "E&xit");
    SetMenuDefaultItem(gui.hmenuTray, IDM_RESTORE, FALSE);

    gui.y_ratio = (70<<10)/100;
    gui.x_ratio = (40<<10)/100;

    gui.mode |= mode_scroll;

    return TRUE;
}


void static guiOnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify){

    switch(id){
    case IDM_EXIT:
        guiOnClose(hwnd);
        break;
    case IDM_SAVE:
        server_save_wraper();
        break;
    case IDM_RESTART:
        server_restart_wraper();
        break;
    case IDM_SHUTDOWN:
        server_quit_wraper();
        break;
    case IDM_SCROLL:
        gui.mode ^= mode_scroll;
        {
            MENUITEMINFO mi;
            mi.cbSize = sizeof(mi);
            mi.fMask = MIIM_STATE;
            mi.fState = (gui.mode&mode_scroll)?MFS_CHECKED:MFS_UNCHECKED;
            mi.wID = IDM_SCROLL;
            SetMenuItemInfo( GetMenu(gui.hwnd), IDM_SCROLL, FALSE, &mi);
        }
        break;
    case IDM_CLEAR:
        guiClearLogWindow();
        break;
    case IDM_RESTORE:
        guiOnShellNotify(IDI_TRAY, WM_LBUTTONDBLCLK);
        break;
    }

}


void static guiOnMenuSelect(HWND hwnd, HMENU hmenu, int item, HMENU hmenuPopup, UINT flags){
 char str[256];

    if( item == 0 || flags == -1)
        SetWindowText( gui.hwndStatus, gui.szDefaultStatus );
    else{
        LoadString( GetWindowInstance(hwnd), item, str, sizeof(str) );
        if( str[0] ) SetWindowText( gui.hwndStatus, str );
    }
}


int static guiOnShellNotify(int uID, int uMessage){

    if(uID == IDI_TRAY){
        if(uMessage == WM_LBUTTONDBLCLK){
            if( !IsWindowVisible(gui.hwnd) )
                ShowWindow(gui.hwnd, SW_RESTORE);
            SetForegroundWindow(gui.hwnd);
        }else if(uMessage == WM_RBUTTONDOWN){
         POINT cp;
            GetCursorPos(&cp);
            SetForegroundWindow(gui.hwnd);
            TrackPopupMenu(gui.hmenuTray, TPM_LEFTALIGN|TPM_LEFTBUTTON, cp.x, cp.y,
                0, gui.hwnd, NULL);

        }

    }
 return 0;
}


void static guiOnPaint(HWND hwnd){
 PAINTSTRUCT ps;
 HDC dc;

    dc = BeginPaint(hwnd, &ps);

    DrawEdge(dc, &gui.rectHDivider, EDGE_SUNKEN, BF_MIDDLE);
    DrawEdge(dc, &gui.rectConsoleEdge, EDGE_SUNKEN, BF_RECT);
    DrawEdge(dc, &gui.rectUsersEdge, EDGE_SUNKEN, BF_RECT);

    EndPaint(hwnd, &ps);

    UpdateWindow(gui.hwndConsole);
    UpdateWindow(gui.hwndUsers);
    UpdateWindow(gui.hwndStatus);
}


void static guiOnClose(HWND hwnd){

    guiKillTrayIcon();
    if( !gui.main_finished ){
     eventlog(eventlog_level_info,"WinMain","GUI wants server dead...");
     exit(0);
    }else{
     eventlog(eventlog_level_info,"WinMain","GUI wants to exit...");
     PulseEvent(gui.event_ready);
    }

}


void static guiOnSize(HWND hwnd, UINT state, int cx, int cy){
 int cy_console, cy_edge, cx_edge, cy_frame, cy_status;
 RECT rs;

    if( state == SIZE_MINIMIZED ){
     NOTIFYICONDATA dta;

        dta.cbSize = sizeof(NOTIFYICONDATA);
        dta.hWnd = hwnd;
        dta.uID = IDI_TRAY;
        dta.uFlags = NIF_ICON|NIF_MESSAGE|NIF_TIP;
        dta.uCallbackMessage = WM_SHELLNOTIFY;
        dta.hIcon = LoadIcon(GetWindowInstance(hwnd), MAKEINTRESOURCE(IDR_ICON16X16));
        strcpy(dta.szTip, "BnetD "BNETD_VERSION);

        Shell_NotifyIcon(NIM_ADD, &dta);
        ShowWindow(hwnd, SW_HIDE);
        return;
    }

    SendMessage( gui.hwndStatus, WM_SIZE, 0, 0);
    GetWindowRect( gui.hwndStatus, &rs );
    cy_status = rs.bottom - rs.top;

    cy_edge = GetSystemMetrics(SM_CYEDGE);
    cx_edge = GetSystemMetrics(SM_CXEDGE);
    cy_frame = (cy_edge<<1) + GetSystemMetrics(SM_CYBORDER) + 1;

    cy_console = ((cy-cy_status-cy_frame-cy_edge*2)*gui.y_ratio)>>10;

    gui.rectConsoleEdge.left = 0;
    gui.rectConsoleEdge.right = cx;
    gui.rectConsoleEdge.top = 0;
    gui.rectConsoleEdge.bottom = cy_edge + cy_console + cy_edge;

    gui.rectConsole.left = cx_edge;
    gui.rectConsole.right = cx - cx_edge;
    gui.rectConsole.top = cy_edge;
    gui.rectConsole.bottom = cy_edge + cy_console;

    gui.rectHDivider.left = 0;
    gui.rectHDivider.top = gui.rectConsole.bottom;
    gui.rectHDivider.right = cx;
    gui.rectHDivider.bottom = gui.rectConsole.bottom + cy_frame;

    gui.rectUsersEdge.left = 0;
    gui.rectUsersEdge.top = gui.rectHDivider.bottom - cy_edge;
    gui.rectUsersEdge.right = cx;
    gui.rectUsersEdge.bottom = cy - cy_status;

    gui.rectUsers.left = cx_edge;
    gui.rectUsers.right = cx - cx_edge;
    gui.rectUsers.top = gui.rectHDivider.bottom;
    gui.rectUsers.bottom = cy - cy_edge - cy_status;

    guiMoveWindow(gui.hwndConsole, &gui.rectConsole);
    guiMoveWindow(gui.hwndUsers, &gui.rectUsers);
}


BOOL static guiOnSetCursor(HWND hwnd, HWND hwndCursor, UINT codeHitTest, UINT msg){
POINT p;

    if(hwnd == hwndCursor && codeHitTest == HTCLIENT){
        GetCursorPos(&p);
        ScreenToClient(hwnd, &p);
        if(PtInRect(&gui.rectHDivider, p)) SetCursor(LoadCursor(0, IDC_SIZENS));
        return TRUE;
    }

    return FORWARD_WM_SETCURSOR(hwnd, hwndCursor, codeHitTest, msg, DefWindowProc);
}


void static guiOnLButtonDown(HWND hwnd, BOOL fDoubleClick, int x, int y, UINT keyFlags){
 POINT p;

    p.x = x;
    p.y = y;

    if( PtInRect(&gui.rectHDivider, p) ){
        SetCapture(hwnd);
        gui.mode |= mode_vdivide;
    }
}


void static guiOnLButtonUp(HWND hwnd, int x, int y, UINT keyFlags){
    ReleaseCapture();
    gui.mode &= ~(mode_vdivide);
}


void static guiOnCaptureChanged(HWND hwndNewCapture){
    gui.mode &= ~(mode_vdivide);
}


void static guiOnMouseMove(HWND hwnd, int x, int y, UINT keyFlags){
 int offset, cy_console, cy_users;
 RECT r;

    if( gui.mode & mode_vdivide ){
        offset = y - gui.rectHDivider.top;
        if( !offset ) return;
        cy_console = gui.rectConsole.bottom - gui.rectConsole.top;
        cy_users = gui.rectUsers.bottom - gui.rectUsers.top;

        if( cy_console + offset <= 0)
            offset = -cy_console;
        else if( cy_users - offset <= 0)
            offset = cy_users;

        cy_console += offset;
        cy_users -= offset;
        if( cy_console + cy_users == 0 ) return;
        gui.y_ratio = (cy_console<<10) / (cy_console + cy_users);
        GetClientRect(hwnd, &r);
        guiOnSize(hwnd, 0, r.right, r.bottom);
        InvalidateRect(hwnd, NULL, FALSE);
    }

}


int extern gui_printf(const char *format, ...){
 va_list arglist;
    va_start(arglist, format);
    return gui_lvprintf(eventlog_level_error, format, arglist);
}


int extern gui_lprintf(t_eventlog_level l, const char *format, ...){
 va_list arglist;
    va_start(arglist, format);
    return gui_lvprintf(l, format, arglist);
}


int extern gui_lvprintf(t_eventlog_level l, const char *format, va_list arglist){
 char buff[4096];
 int result, effects;
 COLORREF clr;

    result = vsprintf(buff, format, arglist);

    switch(l){
    case eventlog_level_none:
      clr = RGB(0, 0, 0);
      effects = CFE_UNDERLINE;
      break;
    case eventlog_level_trace:
      clr = RGB(255, 0, 255);
      effects = 0;
      break;
    case eventlog_level_debug:
      clr = RGB(0, 0, 255);
      effects = 0;
      break;
    case eventlog_level_info:
      clr = RGB(0, 0, 0);
      effects = 0;
      break;
    case eventlog_level_warn:
      clr = RGB(255, 128, 64);
      effects = 0;
      break;
    case eventlog_level_error:
      clr = RGB(255, 0, 0);
      effects = 0;
      break;
    eventlog_level_fatal:
      clr = RGB(255, 0, 0);
      effects = CFE_BOLD;
      break;
    default:
      clr = RGB(0, 0, 0);
    }

    guiAddText(buff, clr, effects);
    return result;
}


void static guiAddText(const char *str, COLORREF clr, int effects){
 int text_length, l1, l2;
 CHARRANGE cr;
 CHARFORMAT fmt;

    SendMessage(gui.hwndConsole, WM_SETREDRAW, FALSE, 0);
    l1 = SendMessage(gui.hwndConsole, EM_GETLINECOUNT, 0, 0);

    text_length = SendMessage(gui.hwndConsole, WM_GETTEXTLENGTH, 0, 0);
    cr.cpMin = text_length;
    cr.cpMax = text_length;
    SendMessage(gui.hwndConsole, EM_EXSETSEL, 0, (LPARAM)&cr);

    fmt.cbSize = sizeof(CHARFORMAT);
    fmt.dwMask = CFM_COLOR|CFM_FACE|CFM_SIZE|
                    CFM_BOLD|CFM_ITALIC|CFM_STRIKEOUT|CFM_UNDERLINE;
    fmt.yHeight = 160;
    fmt.dwEffects = effects;
    fmt.crTextColor = clr;
    strcpy(fmt.szFaceName,"Courier New");

    SendMessage(gui.hwndConsole, EM_SETCHARFORMAT, SCF_SELECTION, (LPARAM)&fmt);
    SendMessage(gui.hwndConsole, EM_REPLACESEL, 0, (LPARAM)str);
    l2 = SendMessage(gui.hwndConsole, EM_GETLINECOUNT, 0, 0);

    SendMessage(gui.hwndConsole, EM_EXSETSEL, 0, (LPARAM)&cr);
    SendMessage(gui.hwndConsole, WM_SETREDRAW, TRUE, 0);

    if( str[strlen(str)-1] == '\n' ){
        InvalidateRect(gui.hwndConsole, NULL, FALSE);
        SendMessage(gui.hwndConsole, EM_LINESCROLL, 0, l2-l1);
    }

    gui.console_size += strlen(str);
    if(gui.console_size >= CONSOLE_MAX_SIZE ){
        SendMessage(gui.hwndConsole, WM_SETTEXT, 0, 0);
        gui.console_size = 0;
    }

}


void static guiDEAD(char *message){
 char *nl;
 char errorStr[4096];
 char *msgLastError;

    FormatMessage(
     FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
     NULL,
     GetLastError(),
     MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
     (LPTSTR) &msgLastError,
     0,
     NULL);

    nl = strchr(msgLastError, '\r');
    if(nl) *nl = 0;

    sprintf(errorStr,
     "%s\n"
     "GetLastError() = '%s'\n",
     message, msgLastError);

    LocalFree(msgLastError);
    MessageBox(0, errorStr, "guiDEAD", MB_ICONSTOP|MB_OK);
    exit(1);
}


void static guiMoveWindow(HWND hwnd, RECT* r){
    MoveWindow(hwnd, r->left, r->top, r->right-r->left, r->bottom-r->top, TRUE);
}


void static guiClearLogWindow(void){
    SendMessage(gui.hwndConsole, WM_SETTEXT, NULL, NULL);
}


void static guiKillTrayIcon(void){
 NOTIFYICONDATA dta;

    dta.cbSize = sizeof(NOTIFYICONDATA);
    dta.hWnd = gui.hwnd;
    dta.uID = IDI_TRAY;
    dta.uFlags = 0;
    Shell_NotifyIcon(NIM_DELETE, &dta);
}


void static guiInsertColumn(HWND view, int n, char *name, int width){
    LV_COLUMN col;
    col.mask = LVCF_TEXT|LVCF_WIDTH|LVCF_SUBITEM;
    col.iSubItem = n;
    col.cx = width;
    col.cchTextMax = strlen(name);
    col.pszText = name;
    SendMessage(view, LVM_INSERTCOLUMN, n, (LPARAM)&col);
}
