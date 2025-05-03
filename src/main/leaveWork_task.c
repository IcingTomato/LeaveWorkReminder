#include <windows.h>
#include <stdio.h>
#include <time.h>
#include <commctrl.h>
#include <uxtheme.h>
#include <gdiplus.h>     // 添加GDI+支持
#include <shlobj.h>      // 获取特殊文件夹路径
#include <Windowsx.h>    // 使用GDI+函数

#pragma comment(lib, "gdiplus.lib")

#define COUNTDOWN_SECONDS 120 // 倒计时总秒数（2分钟）
#define TIMER_ID 1
#define WM_APP_INIT_COUNTDOWN (WM_APP + 1)

void SimulateCtrlS(void);
void CaptureScreenToDesktop(void);
int GetEncoderClsid(const WCHAR* format, CLSID* pClsid);

// 倒计时窗口数据
typedef struct {
    int remainingSeconds;
    HWND hProgressBar;
    HWND hLabel;
} CountdownData;

// 窗口过程函数
LRESULT CALLBACK CountdownWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    static CountdownData* pData = NULL;
    static HFONT hFont = NULL;
    static HBRUSH hLabelBrush = NULL; // 添加标签背景画刷

    switch (msg)
    {
        case WM_CREATE:
            // 分配数据结构
            pData = (CountdownData*)malloc(sizeof(CountdownData));
            if (pData)
            {
                pData->remainingSeconds = COUNTDOWN_SECONDS;
                
                // 创建倒计时标签 - 添加 SS_NOTIFY 样式以接收颜色消息
                pData->hLabel = CreateWindow(
                    "STATIC", "", 
                    WS_CHILD | WS_VISIBLE | SS_CENTER | SS_NOTIFY,
                    50, 20, 200, 40, 
                    hwnd, NULL, GetModuleHandle(NULL), NULL
                );

                // 创建进度条 - 使用Windows 7样式
                pData->hProgressBar = CreateWindowEx(
                    0, PROGRESS_CLASS, NULL, 
                    WS_CHILD | WS_VISIBLE | PBS_SMOOTH,
                    50, 70, 200, 20, 
                    hwnd, NULL, GetModuleHandle(NULL), NULL
                );
                
                // 设置进度条范围
                SendMessage(pData->hProgressBar, PBM_SETRANGE, 0, MAKELPARAM(0, COUNTDOWN_SECONDS));
                SendMessage(pData->hProgressBar, PBM_SETPOS, COUNTDOWN_SECONDS, 0);
                
                // 设置进度条为Windows 7样式
                SendMessage(pData->hProgressBar, PBM_SETSTATE, PBST_NORMAL, 0);
                
                // 应用Visual Styles到进度条
                SetWindowTheme(pData->hProgressBar, L"", L"");
                
                // 创建大字体
                hFont = CreateFont(16, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, 
                                  DEFAULT_CHARSET, OUT_OUTLINE_PRECIS, CLIP_DEFAULT_PRECIS, 
                                  CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE, "Arial");
                
                if (hFont) {
                    SendMessage(pData->hLabel, WM_SETFONT, (WPARAM)hFont, TRUE);
                }
                
                // 发送初始化消息
                PostMessage(hwnd, WM_APP_INIT_COUNTDOWN, 0, 0);
                hLabelBrush = CreateSolidBrush(RGB(255, 255, 255));
            }
            break;
            
        case WM_APP_INIT_COUNTDOWN:
             // 模拟Ctrl+S保存当前工作
            SimulateCtrlS();
    
            // 截取屏幕并保存到桌面
            CaptureScreenToDesktop();

            // 开始计时器
            SetTimer(hwnd, TIMER_ID, 1000, NULL);
            
            // 立即更新显示
            if (pData) {
                char text[128];
                sprintf(text, "还有 %d 秒就要润了!", pData->remainingSeconds);
                SetWindowText(pData->hLabel, text);
            }
            break;
            
        case WM_TIMER:
            if (wParam == TIMER_ID && pData)
            {
                // 更新剩余时间
                pData->remainingSeconds--;
                
                // 更新进度条
                SendMessage(pData->hProgressBar, PBM_SETPOS, pData->remainingSeconds, 0);
                
                // 当剩余时间少于1/3时，将进度条设置为黄色警告状态
                if (pData->remainingSeconds <= COUNTDOWN_SECONDS / 3 && 
                    pData->remainingSeconds > COUNTDOWN_SECONDS / 6) {
                    SendMessage(pData->hProgressBar, PBM_SETSTATE, PBST_PAUSED, 0);
                }
                // 当剩余时间少于1/6时，将进度条设置为红色错误状态
                else if (pData->remainingSeconds <= COUNTDOWN_SECONDS / 6) {
                    SendMessage(pData->hProgressBar, PBM_SETSTATE, PBST_ERROR, 0);
                }
                
                // 更新标签文本
                char text[128];
                sprintf(text, "还有 %d 秒就要润了!", pData->remainingSeconds);
                SetWindowText(pData->hLabel, text);
                
                // 检查是否倒计时完成
                if (pData->remainingSeconds <= 0)
                {
                    KillTimer(hwnd, TIMER_ID);
                    MessageBox(hwnd, "开润！\n请立即关闭计算机！", "警告", MB_ICONERROR | MB_OK);
                    DestroyWindow(hwnd);
                }
            }
            break;
            
        case WM_CTLCOLORSTATIC:
            // 为静态控件(标签)设置自定义颜色
            if ((HWND)lParam == pData->hLabel && hLabelBrush)
            {
                HDC hdcStatic = (HDC)wParam;
                SetTextColor(hdcStatic, RGB(0, 0, 0));    // 黑色文本
                SetBkColor(hdcStatic, RGB(255, 255, 255)); // 白色背景
                return (LRESULT)hLabelBrush;
            }
            break;
            
        case WM_DESTROY:
            if (pData) {
                free(pData);
                pData = NULL;
            }
            if (hFont) {
                DeleteObject(hFont);
                hFont = NULL;
            }
            if (hLabelBrush) {
                DeleteObject(hLabelBrush);
                hLabelBrush = NULL;
            }
            PostQuitMessage(0);
            break;
            
        default:
            return DefWindowProc(hwnd, msg, wParam, lParam);
    }
    return 0;
}

// 模拟Ctrl+S保存操作
void SimulateCtrlS() 
{
    // 准备按键事件
    INPUT inputs[4] = {0};
    
    // 按下Ctrl键
    inputs[0].type = INPUT_KEYBOARD;
    inputs[0].ki.wVk = VK_CONTROL;
    
    // 按下S键
    inputs[1].type = INPUT_KEYBOARD;
    inputs[1].ki.wVk = 'S';
    
    // 释放S键
    inputs[2].type = INPUT_KEYBOARD;
    inputs[2].ki.wVk = 'S';
    inputs[2].ki.dwFlags = KEYEVENTF_KEYUP;
    
    // 释放Ctrl键
    inputs[3].type = INPUT_KEYBOARD;
    inputs[3].ki.wVk = VK_CONTROL;
    inputs[3].ki.dwFlags = KEYEVENTF_KEYUP;
    
    // 发送按键事件
    SendInput(4, inputs, sizeof(INPUT));
    
    // 稍微等待一下让保存操作完成
    Sleep(500);
}

// 截取全屏并保存到桌面
void CaptureScreenToDesktop() 
{
    // 临时启用DPI感知
    BOOL wasPerMonitorDpiAware = FALSE;
    HMODULE hUser32 = LoadLibrary("user32.dll");
    if (hUser32) 
    {
        typedef BOOL(WINAPI* AreDpiAwarenessContextsEqualFunc)(DPI_AWARENESS_CONTEXT, DPI_AWARENESS_CONTEXT);
        typedef DPI_AWARENESS_CONTEXT(WINAPI* GetThreadDpiAwarenessContextFunc)();
        typedef DPI_AWARENESS_CONTEXT(WINAPI* SetThreadDpiAwarenessContextFunc)(DPI_AWARENESS_CONTEXT);
        
        AreDpiAwarenessContextsEqualFunc areDpiAwarenessContextsEqual = 
            (AreDpiAwarenessContextsEqualFunc)GetProcAddress(hUser32, "AreDpiAwarenessContextsEqual");
        GetThreadDpiAwarenessContextFunc getThreadDpiAwarenessContext = 
            (GetThreadDpiAwarenessContextFunc)GetProcAddress(hUser32, "GetThreadDpiAwarenessContext");
        SetThreadDpiAwarenessContextFunc setThreadDpiAwarenessContext = 
            (SetThreadDpiAwarenessContextFunc)GetProcAddress(hUser32, "SetThreadDpiAwarenessContext");
            
        if (areDpiAwarenessContextsEqual && getThreadDpiAwarenessContext && setThreadDpiAwarenessContext) 
        {
            DPI_AWARENESS_CONTEXT previousContext = getThreadDpiAwarenessContext();
            wasPerMonitorDpiAware = areDpiAwarenessContextsEqual(previousContext, DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);
            if (!wasPerMonitorDpiAware) 
            {
                setThreadDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);
            }
        }
    }
    
    WCHAR desktopPath[MAX_PATH];
    WCHAR fileName[MAX_PATH];
    time_t now;
    struct tm *local_time;
    
    // 获取当前时间
    time(&now);
    local_time = localtime(&now);
    
    // 获取桌面路径
    if (SUCCEEDED(SHGetFolderPathW(NULL, CSIDL_DESKTOP, NULL, 0, desktopPath))) 
    {
        // 创建文件名 (格式: YYYY-MM-DD.png)
        swprintf(fileName, MAX_PATH, L"%s\\%04d-%02d-%02d.png", 
                 desktopPath, 
                 local_time->tm_year+1900, 
                 local_time->tm_mon+1, 
                 local_time->tm_mday);
        
        // 初始化GDI+
        ULONG_PTR gdiplusToken;
        GdiplusStartupInput gdiplusStartupInput;
        ZeroMemory(&gdiplusStartupInput, sizeof(GdiplusStartupInput));
        gdiplusStartupInput.GdiplusVersion = 1;
        GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);
        
        // 获取虚拟屏幕尺寸（所有显示器的组合区域）
        int screenWidth = GetSystemMetrics(SM_CXVIRTUALSCREEN);
        int screenHeight = GetSystemMetrics(SM_CYVIRTUALSCREEN);
        int screenLeft = GetSystemMetrics(SM_XVIRTUALSCREEN);
        int screenTop = GetSystemMetrics(SM_YVIRTUALSCREEN);
        
        // 创建兼容DC和位图
        HDC hdcScreen = GetDC(NULL);
        HDC hdcMemDC = CreateCompatibleDC(hdcScreen);
        HBITMAP hbmScreen = CreateCompatibleBitmap(hdcScreen, screenWidth, screenHeight);
        SelectObject(hdcMemDC, hbmScreen);
        
        // 复制屏幕内容到位图，使用虚拟屏幕坐标
        BitBlt(hdcMemDC, 0, 0, screenWidth, screenHeight, 
               hdcScreen, screenLeft, screenTop, SRCCOPY);
        
        // 使用GDI+保存图像
        CLSID pngClsid;
        GetEncoderClsid(L"image/png", &pngClsid);
        
        // 使用C风格的GDI+ API
        GpBitmap* bitmap;
        GdipCreateBitmapFromHBITMAP(hbmScreen, NULL, &bitmap);
        GdipSaveImageToFile(bitmap, fileName, &pngClsid, NULL);
        GdipDisposeImage(bitmap);
        
        // 清理
        ReleaseDC(NULL, hdcScreen);
        DeleteDC(hdcMemDC);
        DeleteObject(hbmScreen);
        GdiplusShutdown(gdiplusToken);

        // 恢复原来的DPI感知设置
        if (hUser32 && !wasPerMonitorDpiAware) 
        {
            typedef DPI_AWARENESS_CONTEXT(WINAPI* SetThreadDpiAwarenessContextFunc)(DPI_AWARENESS_CONTEXT);
            SetThreadDpiAwarenessContextFunc setThreadDpiAwarenessContext = 
                (SetThreadDpiAwarenessContextFunc)GetProcAddress(hUser32, "SetThreadDpiAwarenessContext");
            if (setThreadDpiAwarenessContext) 
            {
                setThreadDpiAwarenessContext(DPI_AWARENESS_CONTEXT_UNAWARE);
            }
        }
        
        if (hUser32) 
        {
            FreeLibrary(hUser32);
        }
    }
}

// 获取编码器CLSID的辅助函数
int GetEncoderClsid(const WCHAR* format, CLSID* pClsid) 
{
    UINT num = 0;          // 编码器数量
    UINT size = 0;         // 编码器数组大小
    
    // 获取图像编码器数量和大小
    GetImageEncodersSize(&num, &size);
    if(size == 0) return -1;
    
    // 分配内存
    ImageCodecInfo* pImageCodecInfo = (ImageCodecInfo*)(malloc(size));
    if(pImageCodecInfo == NULL) return -1;
    
    // 获取编码器信息
    GetImageEncoders(num, size, pImageCodecInfo);
    
    // 寻找匹配的编码器
    for(UINT j = 0; j < num; ++j) 
    {
        if(wcscmp(pImageCodecInfo[j].MimeType, format) == 0) 
        {
            *pClsid = pImageCodecInfo[j].Clsid;
            free(pImageCodecInfo);
            return j;
        }
    }
    
    free(pImageCodecInfo);
    return -1;
}

// 主函数
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    // 写入日志，证明程序确实启动了
    FILE *logFile = fopen(".\\LeaveWork_log.txt", "a");
    if (logFile) 
    {
        time_t now;
        struct tm *local_time;
        time(&now);
        local_time = localtime(&now);
        
        fprintf(logFile, "[%04d-%02d-%02d %02d:%02d:%02d] 程序启动\n", 
               local_time->tm_year+1900, local_time->tm_mon+1, local_time->tm_mday,
               local_time->tm_hour, local_time->tm_min, local_time->tm_sec);
        fclose(logFile);
    }
    
    // 确保Common Controls DLL已初始化
    INITCOMMONCONTROLSEX icc;
    icc.dwSize = sizeof(INITCOMMONCONTROLSEX);
    icc.dwICC = ICC_PROGRESS_CLASS | ICC_STANDARD_CLASSES;
    InitCommonControlsEx(&icc);
    
    // 注册窗口类
    WNDCLASSEX wc = {0};
    wc.cbSize = sizeof(WNDCLASSEX);
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = CountdownWndProc;
    wc.hInstance = hInstance;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW+1);
    wc.lpszClassName = "CountdownWindowClass";
    RegisterClassEx(&wc);
    
    // 创建窗口
    HWND hWnd = CreateWindowEx(
        WS_EX_TOPMOST | WS_EX_TOOLWINDOW,
        "CountdownWindowClass",
        "下班倒计时",
        WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,
        CW_USEDEFAULT, CW_USEDEFAULT, 300, 150,
        NULL, NULL, hInstance, NULL
    );
    
    if (hWnd)
    {
        // 将窗口居中显示
        RECT rc;
        GetWindowRect(hWnd, &rc);
        int screenWidth = GetSystemMetrics(SM_CXSCREEN);
        int screenHeight = GetSystemMetrics(SM_CYSCREEN);
        int windowWidth = rc.right - rc.left;
        int windowHeight = rc.bottom - rc.top;
        SetWindowPos(hWnd, NULL, 
                     (screenWidth - windowWidth) / 2, 
                     (screenHeight - windowHeight) / 2,
                     0, 0, SWP_NOSIZE | SWP_NOZORDER);
        
        // 显示窗口
        ShowWindow(hWnd, SW_SHOW);
        UpdateWindow(hWnd);
        
        // 消息循环
        MSG msg;
        while (GetMessage(&msg, NULL, 0, 0))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }
    
    return 0;
}