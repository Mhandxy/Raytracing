#include "XY.h"

int cwsw;
int cwsh;
HDC hdc;
XYCamera* camera;
XYScene* scene;
XYScreenBuffer* buffer;
XYLight* light;
XYRayI envcolor;
int Frame=0;

//MODE
XYMODE M_AA=XY_AA_CLOSE;//antialiasing
XYMODE M_DOF=XY_DOF_CLOSE;//depth of field
XYMODE M_SHA=XY_SHA_CLOSE;//soft shadow

void XYInit(HDC _hdc)
{
    RECT rt;  
    HWND hWnd=WindowFromDC(_hdc);  
    GetClientRect(hWnd, &rt);
    hdc=_hdc;
    envcolor=XYRayI(0.2f,0.2f,0.2f);
    cwsw=rt.right-rt.left;
    cwsh=rt.bottom-rt.top;
    buffer=new XYScreenBuffer(hdc);
    scene=new XYScene(20);
    //camera=new XYCamera(cwsw,cwsh,XYVector3(20,6,3),XYVector3(-10,-2,2));
    camera=new XYCamera(cwsw,cwsh,XYVector3(10,0,0),XYVector3(-8,0,0));
    light=new XYLight();
    //light->AddLight(new XYLightSource(XY_LT_POINT,XYVector3(5,-5,10)));
    light->AddLight(new XYLightSource(XY_LT_POINT,XYVector3(5,5,10)));
    
    scene->AddObj(new XYPlane(VEC_ORIGIN,120,40,XY_MAT_PLA_2));
    
    scene->AddObj(new XYPlane(XYVector3(-20,0,10),120,20,XY_MAT_PLA_3,0,Pi/2));
    scene->AddObj(new XYBox(XYVector3(-4,0,0.1f),0.4f,8.2f,0.1f,XY_MAT_BOX_0));
    scene->AddObj(new XYPlane(XYVector3(-4,0,3.2f),8,6,XY_MAT_PLA_4,0,Pi/2));
    
    XYSetMode(XY_AA_OPEN);
    XYSetMode(XY_SHA_SOFT);
    //XYSetMode(XY_DOF_OPEN);
    for(int i=2;i<2;i++)
    {
        for(int j=-2;j<3;j++)
        {
            for(int k=0;k<4;k++)
            {
                scene->AddObj(new XYSphere(XYVector3(-2.0f*i+2.0f,2.0f*j,2.0f*k+0.5f),0.5f,XY_MAT_SPH_2));
            }
        }
    }
    CSimpleObject obj;
    obj.LoadFromObj("dinosaur.obj");
    if(obj.IsLoaded())
        scene->AddMeshObj(&obj,XYVector3(-2,0,1.25),3);
    CSimpleObject obj2;
    obj2.LoadFromObj("lxy.obj");
    if(obj2.IsLoaded())
        scene->AddMeshObj(&obj2,XYVector3(5,2,0.55),2);
    //scene->bsptree.SaveToObj("bunny.fine_bsptree.obj");
    Frame=21;
    scene->AddObj(new XYSphere(XYVector3(2,-2,1.501f),1,XY_MAT_SPH_3));
    scene->AddObj(new XYBox(XYVector3(2,-2,0.2505f),2,2,0.5,XY_MAT_BOX_1));
    scene->AddObj(new XYSphere(XYVector3(0,0,0.5f),0.5,XY_MAT_SPH_2));
    scene->AddObj(new XYSphere(XYVector3(1,2,1),1,XY_MAT_SPH_1));

}

void XYRender()
{
    if(Frame<=80)
        camera->Lookat(XYVector3(-10*sinf(float(Frame)/180*Pi+Pi*9/8),10*cosf(float(Frame)/180*Pi+Pi*9/8),2),XYVector3(-4,0,0));
    else if(Frame>90)
        PostQuitMessage(0);
    static float t1=0.2f;
    if(abs(camera->F-20)>=10) t1=-t1;
    //camera->F-=t1;
    camera->Image(scene,buffer);
    Frame+=1;
    //if(Frame>10&&camera->F==10)
        //PostQuitMessage(0);
}

void XYRelease()
{
    SAFE_RELEASE(buffer);
    SAFE_RELEASE(scene);
    SAFE_RELEASE(camera);
    SAFE_RELEASE(light);
}

XYRayI XYGetEnvCol()
{
    return envcolor;
}

XYLight* XYGetLight()
{
    return light;
}

XYScene* XYGetScene()
{
    return scene;
}

XYMODE XYGetMode(XYMODE i)
{
    switch(i)
    {
    case XY_MODE_AA:
        return M_AA;
        break;
    case XY_MODE_DOF:
        return M_DOF;
        break;
    case XY_MODE_SHA:
        return M_SHA;
        break;
    }
    return 0;
}

XYMODE XYSetMode(XYMODE i)
{
    switch(i)
    {
    case XY_AA_CLOSE:
        M_AA=XY_AA_CLOSE;
        camera->Init();
        break;
    case XY_AA_OPEN:
        M_AA=XY_AA_OPEN;
        camera->Init();
        break;
    case XY_DOF_CLOSE:
        M_DOF=XY_DOF_CLOSE;
        break;
    case XY_DOF_OPEN:
        M_DOF=XY_DOF_OPEN;
        break;
    case XY_SHA_CLOSE:
        M_SHA=XY_SHA_CLOSE;
        break;
    case XY_SHA_SOFT:
        M_SHA=XY_SHA_SOFT;
        break;
    case XY_SHA_HARD:
        M_SHA=XY_SHA_HARD;
        break;
    }
    return 0;
}

//////////////////////////////////////////////////
XYScreenBuffer::XYScreenBuffer(HDC _hdc)
    :hdc(_hdc)
{
    HWND hWnd=WindowFromDC(hdc);  
    RECT rt;
    GetClientRect(hWnd, &rt);
    int tmpw=rt.right-rt.left;
    int tmph=rt.bottom-rt.top;

    hmemdc=CreateCompatibleDC(hdc);
    membm=CreateCompatibleBitmap(hdc,2*tmpw,2*tmph);
    SelectObject(hmemdc,membm);
    DeleteObject(membm);
    BITMAP bm;
    GetObject(membm,sizeof(BITMAP),&bm);
    buffer=new BYTE[bm.bmHeight*bm.bmWidth*bm.bmBitsPixel];
}

XYScreenBuffer::~XYScreenBuffer()
{
    DeleteDC(hmemdc);
    SAFE_RELEASE(buffer);
}

BYTE* XYScreenBuffer::GetBuffer()
{
    return buffer;
}

HDC XYScreenBuffer::GetMemDC()
{
    return hmemdc; 
}

HDC XYScreenBuffer::GetHDC()
{
    return hdc;
}