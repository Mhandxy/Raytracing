#include "XY.h"

XYLightSource::XYLightSource(LightType _type,XYVector3& _para,XYRayI& _ill)
    :para(_para),ill(_ill),type(_type)
{

}

XYVector3 XYLightSource::GetDir(XYVector3& P)
{
    switch(type)
    {
    case XY_LT_POINT:
        return para-P;
    case XY_LT_PARALLEL:
        return -para;
    default:
        return VEC_NULL;
    }
}

XYRayI XYLightSource::GetIll()
{
    return ill;
}

void XYLightSource::GetShadow(XYVector3& P,float& d,float& s)
{    
    XYVector3 L=GetDir(P);
    float dis=L.Length();
    L.Unit();
    float tmin;
    if(d<0)
        s=d=0;
    else if(XYGetMode(XY_MODE_SHA)!=XY_SHA_CLOSE)
    {
        int n=5;
        if(XYGetMode(XY_MODE_SHA)==XY_SHA_HARD)
            n=0;
        int td=0;
        float step=0.0004f;
        float crate=0;
        for(int i=-n;i<=n;++i)
        {
            for(int j=-n;j<=n;++j)
            {
                if(!XYGetScene()->RayIntersection(P,L+i*XYVector3(dis*step,0,0)+j*XYVector3(0,dis*step,0),tmin))
                {
                    if(tmin<dis)
                        td++;
                }
            }
        }
        float rate=float(td)/(2*n+1)/(2*n+1);
        d=d*rate;
        s=s*rate;
    }
    d/=XYGetLight()->GetCount();
    if(type==XY_LT_POINT)
    {
        if(dis>12)
            d*=144/dis/dis;
    }
    if(type==XY_LT_PARALLEL)
    {
        s=0;
    }
}

XYLight::~XYLight()
{
    for(auto iter=light.begin();iter!=light.end();++iter)
    {
        SAFE_RELEASE(*iter);
    }
}

void XYLight::AddLight(XYLightSource* ls)
{
    light.push_back(ls);
}

XYLightSource* XYLight::operator[](int i)
{
    if(i>=0&&i<int(light.size()))
        return light[i];
    else
        return NULL;
}

int XYLight::GetCount()
{
    return light.size();
}
