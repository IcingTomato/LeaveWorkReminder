#include <windows.h>
#include <stdio.h>
#include <time.h>
#include <commctrl.h>
#include <uxtheme.h>

#define COUNTDOWN_SECONDS 120 // 倒计时总秒数（2分钟）
#define TIMER_ID 1
#define WM_APP_INIT_COUNTDOWN (WM_APP + 1)

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
                    MessageBox(hwnd, "开润！\n请立即关闭计算机！", "警告", MB_ICONWARNING | MB_OK);
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

// 主函数
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    // 写入日志，证明程序确实启动了
    FILE *logFile = fopen(".\\LeaveWork_log.txt", "a");
    if (logFile) {
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