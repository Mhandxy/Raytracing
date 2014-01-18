#include "XY.h"

//消息回掉函数
LRESULT CALLBACK WndProc (HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);

//消息处理函数
void ChangeSize(int w,int h);
void MouseMove(int x,int y);
void MouseClick(int x,int y);

//windows主函数
int WINAPI WinMain (HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR szCmdLine, int iCmdShow)
{
    
    int CWSIZEW=600;//当前窗口宽
    int CWSIZEH=600;//当前窗口高
    
    TCHAR AppName[]=TEXT ("光线跟踪");
    WNDCLASSEX   wndclassex ={0};
    wndclassex.cbSize       =sizeof(WNDCLASSEX);
    wndclassex.style        =CS_HREDRAW | CS_VREDRAW;
    wndclassex.lpfnWndProc  =WndProc;
    wndclassex.cbClsExtra   =0;
    wndclassex.cbWndExtra   =0;
    wndclassex.hInstance    =hInstance;
    wndclassex.hIcon        =LoadIcon (NULL, IDI_APPLICATION);
    wndclassex.hCursor      =LoadCursor (NULL, IDC_ARROW);
    wndclassex.hbrBackground=(HBRUSH) GetStockObject (WHITE_BRUSH);
    wndclassex.lpszMenuName =NULL;
    wndclassex.lpszClassName=AppName;
    wndclassex.hIconSm      =wndclassex.hIcon;

    if (!RegisterClassEx (&wndclassex))
    {
        MessageBox (NULL, TEXT ("RegisterClassEx failed!"), AppName, MB_ICONERROR);
        return 0;
    }
    HWND hwnd=CreateWindowEx ( WS_EX_OVERLAPPEDWINDOW,
                            AppName,
                            TEXT ("光线跟踪"),
                            WS_OVERLAPPEDWINDOW&~WS_THICKFRAME&~WS_MAXIMIZEBOX,
                            CW_USEDEFAULT,
                            CW_USEDEFAULT,
                            CWSIZEW,
                            CWSIZEH,
                            NULL,
                            NULL,
                            hInstance,
                            NULL);
    //调整窗口大小
    RECT now;
    GetClientRect(hwnd,&now);
    SetWindowPos(hwnd,NULL,200,100,2*CWSIZEW-now.right,2*CWSIZEH-now.bottom,NULL);
    
    HDC hdc=GetDC(hwnd);
    XYInit(hdc);

    ShowWindow (hwnd, iCmdShow);
    UpdateWindow (hwnd);
    MSG msg={0};

    while(msg.message!=WM_QUIT)
    {
        if(PeekMessage(&msg,NULL,0,0,PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg); 
        }
        else
        {
            XYRender();
        }
    }
    UnregisterClass(AppName,wndclassex.hInstance);
    return (msg.wParam);
}

//消息回调函数
LRESULT CALLBACK WndProc (HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    //case WM_PAINT:
        //XYRender(hdc);
        //return 0;
        /*
    case WM_MOUSEMOVE:
        MouseMove(LOWORD(lParam),HIWORD(lParam));
        break;*/
    case WM_LBUTTONDOWN:
        if(XYGetMode(XY_MODE_AA)==XY_AA_OPEN)
            XYSetMode(XY_AA_CLOSE);
        else
            XYSetMode(XY_AA_OPEN);
        break;
    case WM_RBUTTONDOWN:
        if(XYGetMode(XY_MODE_DOF)==XY_DOF_CLOSE)
            XYSetMode(XY_DOF_OPEN);
        else
            XYSetMode(XY_DOF_CLOSE);
        break;
    case WM_DESTROY:
        PostQuitMessage (0);
        XYRelease();
        break;
    case WM_KEYDOWN:
        if(wParam==VK_ESCAPE)
            PostQuitMessage (0);
        break;
    }
    return DefWindowProc (hwnd, message, wParam, lParam);
}

void MouseMove(int x,int y)
{

}

void MouseClick(int x,int y)
{

}
