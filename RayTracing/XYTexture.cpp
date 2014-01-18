#include "XY.h"

BOOL SaveBitmapToFile(HBITMAP hBitmap, LPSTR lpFileName)
{ 
    HDC hDC; //�豸������ 
    int iBits; //��ǰ��ʾ�ֱ�����ÿ��������ռ�ֽ��� 
    WORD wBitCount; //λͼ��ÿ��������ռ�ֽ��� 
    DWORD dwPaletteSize=0, //�����ɫ���С�� λͼ�������ֽڴ�С ��λͼ�ļ���С �� д���ļ��ֽ��� 
        dwBmBitsSize, 
        dwDIBSize, dwWritten; 
    BITMAP Bitmap; //λͼ���Խṹ 
    BITMAPFILEHEADER bmfHdr; //λͼ�ļ�ͷ�ṹ 
    BITMAPINFOHEADER bi; //λͼ��Ϣͷ�ṹ 
    LPBITMAPINFOHEADER lpbi; //ָ��λͼ��Ϣͷ�ṹ 

    HANDLE fh, hDib, hPal,hOldPal=NULL; //�����ļ��������ڴ�������ɫ���� 

    //����λͼ�ļ�ÿ��������ռ�ֽ��� 
    HDC hWndDC = CreateDC(TEXT("DISPLAY"),NULL,NULL,NULL); 
    hDC = CreateCompatibleDC( hWndDC ) ; 
    iBits = GetDeviceCaps(hDC, BITSPIXEL) * GetDeviceCaps(hDC, PLANES); 
    DeleteDC(hDC); 

    if (iBits <= 1) 
        wBitCount = 1; 
    else if (iBits <= 4) 
        wBitCount = 4; 
    else if (iBits <= 8) 
        wBitCount = 8; 
    else if (iBits <= 24) 
        wBitCount = 24; 
    else 
        wBitCount = 24 ; 

    //�����ɫ���С 
    if (wBitCount <= 8) 
        dwPaletteSize = (1 << wBitCount) * sizeof(RGBQUAD); 

    //����λͼ��Ϣͷ�ṹ 
    GetObject(hBitmap, sizeof(BITMAP), (LPSTR)&Bitmap); 
    bi.biSize = sizeof(BITMAPINFOHEADER); 
    bi.biWidth = Bitmap.bmWidth; 
    bi.biHeight = Bitmap.bmHeight; 
    bi.biPlanes = 1; 
    bi.biBitCount = wBitCount; 
    bi.biCompression = BI_RGB; 
    bi.biSizeImage = 0; 
    bi.biXPelsPerMeter = 0; 
    bi.biYPelsPerMeter = 0; 
    bi.biClrUsed = 0; 
    bi.biClrImportant = 0; 

    dwBmBitsSize = ((Bitmap.bmWidth * wBitCount+31)/32) * 4 * Bitmap.bmHeight ; 

    //Ϊλͼ���ݷ����ڴ� 
    hDib = GlobalAlloc(GHND,dwBmBitsSize+dwPaletteSize+sizeof(BITMAPINFOHEADER)); 
    lpbi = (LPBITMAPINFOHEADER)GlobalLock(hDib); 
    *lpbi = bi; 

    // �����ɫ�� 
    hPal = GetStockObject(DEFAULT_PALETTE); 
    if (hPal) 
    { 
        hDC = ::GetDC(NULL); 
        hOldPal = ::SelectPalette(hDC, (HPALETTE)hPal, FALSE); 
        RealizePalette(hDC); 
    } 

    // ��ȡ�õ�ɫ�����µ�����ֵ 
    GetDIBits(hDC, hBitmap, 0, (UINT) Bitmap.bmHeight, 
        (LPSTR)lpbi + sizeof(BITMAPINFOHEADER) 
        +dwPaletteSize, 
        (LPBITMAPINFO ) 
        lpbi, DIB_RGB_COLORS); 

    //�ָ���ɫ�� 
    if (hOldPal) 
    { 
        SelectPalette(hDC, (HPALETTE)hOldPal, TRUE); 
        RealizePalette(hDC); 
        ::ReleaseDC(NULL, hDC); 
    } 

    //����λͼ�ļ� 
    fh = CreateFileA(lpFileName, GENERIC_WRITE, 
        0, NULL, CREATE_ALWAYS, 
        FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN, NULL); 

    if (fh == INVALID_HANDLE_VALUE) 
        return FALSE; 

    // ����λͼ�ļ�ͷ 
    bmfHdr.bfType = 0x4D42; // "BM" 
    dwDIBSize = sizeof(BITMAPFILEHEADER) 
        + sizeof(BITMAPINFOHEADER) 
        + dwPaletteSize + dwBmBitsSize; 
    bmfHdr.bfSize = dwDIBSize; 
    bmfHdr.bfReserved1 = 0; 
    bmfHdr.bfReserved2 = 0; 
    bmfHdr.bfOffBits = (DWORD)sizeof(BITMAPFILEHEADER) 
        + (DWORD)sizeof(BITMAPINFOHEADER) 
        + dwPaletteSize; 

    // д��λͼ�ļ�ͷ 
    WriteFile(fh, (LPSTR)&bmfHdr, sizeof(BITMAPFILEHEADER), &dwWritten, NULL); 

    // д��λͼ�ļ��������� 
    WriteFile(fh, (LPSTR)lpbi, dwDIBSize, 
        &dwWritten, NULL); 

    //��� 
    GlobalUnlock(hDib); 
    GlobalFree(hDib); 
    CloseHandle(fh); 

    return TRUE; 
}

//////////////////////////////////////////////
XYTexture::XYTexture(LPWSTR lpFileName)
{
    hbitmap=(HBITMAP)LoadImage(0,lpFileName,IMAGE_BITMAP,0,0,LR_LOADFROMFILE|LR_CREATEDIBSECTION);
    BITMAP bitmap;
    GetObject(hbitmap,sizeof(BITMAP),&bitmap);
    bheight=bitmap.bmHeight;
    bwidth=bitmap.bmWidth;
    bit=bitmap.bmBitsPixel/8;
    DWORD dwSize=bitmap.bmHeight*bitmap.bmWidthBytes;
    pBits=new BYTE[dwSize];
    GetBitmapBits(hbitmap,dwSize,pBits);
    DeleteObject(hbitmap);
}

XYTexture::~XYTexture()
{
    SAFE_RELEASE(pBits);
}

COLORREF XYTexture::GetColor(float i,float j,int PARA)
{
    if(hbitmap==NULL)
        return COL_BLACK;
    if(PARA==XY_TEX_REL)
    {
        i*=bwidth;
        j*=bheight;
    }
    int t=bit*(int(i)%bwidth+int(j)%bheight*bwidth);
    return RGB(pBits[t+2],pBits[t+1],pBits[t]);
}