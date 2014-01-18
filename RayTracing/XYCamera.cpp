#include "XY.h"
#include <omp.h>

XYCamera::XYCamera(int w,int h,XYVector3& _o,XYVector3& dir)
    :pixelx(w),pixely(h),height(1.0f),o(_o),d(dir),F(10),L(3)
{
    width=float(w)/h*height;
    if(d==o)
        o=XYVector3(10,0,5);
    ZBuffer=new float[4*pixelx*pixely];
    Init();
}

XYCamera::~XYCamera()
{
    SAFE_RELEASE(ZBuffer);
}

void XYCamera::Init()
{
    static int flag=false;
    if(XYGetMode(XY_MODE_AA)==XY_AA_OPEN)
    {
        pixelx=pixelx*2;
        pixely=pixely*2;
        flag=true;
    }
    else if(flag)
    {
        pixelx=pixelx/2;
        pixely=pixely/2;
    }
    Lookat(o,d);
}

XYVector3 XYCamera::GetOrigin()
{
    return o;
}

void XYCamera::Lookat(XYVector3& _o,XYVector3& dir,XYVector3& up)
{
    o=_o;
    if(SMALL(DotProduct(o-dir,o-dir))) return;
    float p=DotProduct(up,o-dir)/DotProduct(o-dir,o-dir);
    XYVector3 n=-p*(o-dir)+up;
    if(SMALL(n.Length()))
    {
        if(up==XYVector3(0,0,1))
            n=XYVector3(1,0,0);
        else
        {
            up=XYVector3(0,0,1);
            p=DotProduct(up,o-dir)/DotProduct(o-dir,o-dir);
            n=-p*(o-dir)+up;
        }
    }
    XYVector3 r=CrossProduct(dir-o,n);
    ex=Unit(r)/float(pixelx)*width;
    ey=Unit(n)/float(pixely)*height;
    no=o+Unit(dir-o)-(pixelx/2.0f-0.5f)*ex-(pixely/2.0f-0.5f)*ey;
}

COLORREF XYCamera::RayTracing(XYScene* scene,XYVector3& dir,float& tmin)
{
    XYRayI I;
    dir.Unit();
    RayTracing(scene,o,dir,1.0f,I,tmin);
    I.Adjust();
    return RGB(I.r*255,I.g*255,I.b*255);
}

#define MINWEIGHT 0.01f
void XYCamera::RayTracing(XYScene* scene,XYVector3& ro,XYVector3& dir,float weight,XYRayI& I,float& tmin)
{
    if(dir==VEC_NULL)
        return;
    if(weight<MINWEIGHT)
    {
        I=ILL_BLACK;
        return;
    }
    dir.Unit();
    XYVector3 P,R,T;
    XYRayI Il,Ir,It;
    XYRayK Ks,Kt;
    float unuse;
    if(!scene->RayIntersection(ro,dir,P,R,Ks,T,Kt,Il,tmin))
    {
        I=ILL_BLACK;
        return;
    }
    else
    {
        //反射
        if(SMALL(Ks.r)&&SMALL(Ks.g)&&SMALL(Ks.b))
            Ir=ILL_BLACK;
        else
            RayTracing(scene,P,R,weight*(Ks.r+Ks.g+Ks.b)/3.0f,Ir,unuse);
        //折射
        if(SMALL(Kt.r)&&SMALL(Kt.g)&&SMALL(Kt.b))
            It=ILL_BLACK;
        else
            RayTracing(scene,P,T,weight*(Kt.r+Kt.g+Kt.b)/3.0f,It,unuse);
        //Phong模型
        I=Il+Ir*Ks+It*Kt;
        I.Adjust();
        I=I*weight;
    }
}

void XYCamera::Image(XYScene* scene,XYScreenBuffer* buffer)
{
    freopen("log.txt", "a", stdout);
    set_time_base();
    BYTE *buf=buffer->GetBuffer();
    BYTE* img=new BYTE[pixelx*pixely*4];
    int tx=pixelx;
    int ty=pixely;
    //光线跟踪
    #pragma omp parallel for schedule(dynamic,1)
    for(int j=ty-1;j>=0;--j)
    {
        for(int i=0;i<tx;++i)
        {
            float l;
            COLORREF col=RayTracing(scene,no-o+i*ex+j*ey,l);
            if(l==-1)
                l=1000;
            ZBuffer[i+(ty-1-j)*tx]=l*DotProduct(Unit(no-o+i*ex+j*ey),Unit(no-o+0.5f*tx*ex+0.5f*ty*ey));
            int tmp=4*((ty-j-1)*tx+i);
            img[tmp  ]=(BYTE)GetBValue(col);
            img[tmp+1]=(BYTE)GetGValue(col);
            img[tmp+2]=(BYTE)GetRValue(col);
            img[tmp+3]=255;
        }
    }
    //处理景深
    if(XYGetMode(XY_MODE_DOF)==XY_DOF_OPEN)
    {
        BYTE* tmpbuf=new BYTE[tx*ty*4];
        memcpy(tmpbuf,img,tx*ty*4);
        #pragma omp parallel for schedule(dynamic,1)
        for(int j=0;j<ty;++j)
        {
            for(int i=0;i<tx;++i)
            {
                float tmpbyte[3]={0,0,0};
                float sigma=abs(ZBuffer[i+j*tx]-F)/L;
                int tmp=4*(j*tx+i);
                int range=QZ(3*sigma);
                if(range>10) range=10;
                float crate=0;
                for(int m=-range;m<=range;++m)
                {
                    for(int n=-range;n<=range;++n)
                    {
                        crate+=Gauss2(float(m),float(n),abs(ZBuffer[abs(tx-1-abs(tx-1-i-m))+abs(ty-1-abs(ty-1-n-j))*tx])/L);
                    }
                }
                for(int m=-range;m<=range;++m)
                {
                    for(int n=-range;n<=range;++n)
                    {
                        float rate=Gauss2(float(m),float(n),abs(ZBuffer[abs(tx-1-abs(tx-1-i-m))+abs(ty-1-abs(ty-1-n-j))*tx])/L)/crate;
                        tmpbyte[0]+=float(tmpbuf[(abs(tx-1-abs(tx-1-i-m)))*4+4*(abs(ty-1-abs(ty-1-n-j)))*tx])*rate;
                        tmpbyte[1]+=float(tmpbuf[(abs(tx-1-abs(tx-1-i-m)))*4+4*(abs(ty-1-abs(ty-1-n-j)))*tx+1])*rate;
                        tmpbyte[2]+=float(tmpbuf[(abs(tx-1-abs(tx-1-i-m)))*4+4*(abs(ty-1-abs(ty-1-n-j)))*tx+2])*rate;
                    }
                }
                img[tmp]=BYTE(tmpbyte[0]);
                img[tmp+1]=BYTE(tmpbyte[1]);
                img[tmp+2]=BYTE(tmpbyte[2]);
            }
        }
        SAFE_RELEASE(tmpbuf);
    }
    //反锯齿
    if(XYGetMode(XY_MODE_AA)!=XY_AA_OPEN)
    {
        memcpy(buf,img,tx*ty*4);
    }
    else
    {
        tx=tx/2;
        ty=ty/2;
        #pragma omp parallel for schedule(dynamic,1)
        for(int j=0;j<ty;++j)
        {
            for(int i=0;i<tx;++i)
            {
                int tmp=4*(j*tx+i);
                buf[tmp]=BYTE(img[4*(2*i)+8*(2*j)*tx]/4.0f+img[4*(2*i+1)+8*(2*j)*tx]/4.0f+img[4*(2*i)+8*(2*j+1)*tx]/4.0f+img[4*(2*i+1)+8*(2*j+1)*tx]/4.0f);
                buf[tmp+1]=BYTE(img[4*(2*i)+8*(2*j)*tx+1]/4.0f+img[4*(2*i+1)+8*(2*j)*tx+1]/4.0f+img[4*(2*i)+8*(2*j+1)*tx+1]/4.0f+img[4*(2*i+1)+8*(2*j+1)*tx+1]/4.0f);
                buf[tmp+2]=BYTE(img[4*(2*i)+8*(2*j)*tx+2]/4.0f+img[4*(2*i+1)+8*(2*j)*tx+2]/4.0f+img[4*(2*i)+8*(2*j+1)*tx+2]/4.0f+img[4*(2*i+1)+8*(2*j+1)*tx+2]/4.0f);
                buf[tmp+3]=255;
            }
        }
    }
    float ttime=float(get_time_in_seconds());
    //计时
    time_t rawtime;
    struct tm* timeinfo;
    time(&rawtime);
    timeinfo=localtime(&rawtime);
    static char date[40];
    sprintf(date,"%02d%02d%02d%02d%02d",timeinfo->tm_mon+1,timeinfo->tm_mday,timeinfo->tm_hour,timeinfo->tm_min,timeinfo->tm_sec);
    //
    printf("%s %f\n",date,ttime);
    //保存显示位图
    HBITMAP bm=CreateBitmap(tx,ty,1,32,buf);
    DeleteObject(SelectObject(buffer->GetMemDC(),bm));
    static char filename[40];
    sprintf(filename,"%s.bmp",date);
    SaveBitmapToFile(bm,filename);
    DeleteObject(bm);
    BitBlt(buffer->GetHDC(),0,0,tx,ty,buffer->GetMemDC(),0,0,SRCCOPY);

    SAFE_RELEASE(img);
}