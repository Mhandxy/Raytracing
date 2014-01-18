#include "XY.h"

//XYVector3
XYVector3::XYVector3()
    :x(0.0f),y(0.0f),z(0.0f)
{
}

XYVector3::XYVector3(float _x,float _y,float _z)
    :x(_x),y(_y),z(_z)
{
}

XYVector3::XYVector3(float r,float theta,float phi,int unuse)
{
    x=r*sinf(theta)*cosf(phi);
    y=r*sinf(theta)*sinf(phi);
    z=r*cosf(theta);
}

float XYVector3::Length()
{
    return sqrt(x*x+y*y+z*z);
}

void XYVector3::SphCoor(float& r,float& theta,float& phi)
{
    r=Length();
    if(r==0)
    {
        theta=0;
        phi=0;
        return;
    }
    theta=acosf(z/r);
    float rxy=sqrt(x*x+y*y);
    if(rxy==0)
        phi=0;
    else
        phi=acosf(x/rxy);
    if(y<0)
        phi=2*Pi-phi;
}

void XYVector3::Unit()
{
    float tmp=Length();
    x=x/tmp;
    y=y/tmp;
    z=z/tmp;
}

void XYVector3::RotateX(float theta)
{
    float r=sqrt(y*y+z*z);
    if(r==0) return;
    float t=acosf(y/r);
    if(z<0) t=2*Pi-t;
    z=r*sinf(t+theta);
    y=r*cosf(t+theta);
}

void XYVector3::RotateY(float theta)
{
    float r=sqrt(x*x+z*z);
    if(r==0) return;
    float t=acosf(z/r);
    if(x<0) t=2*Pi-t;
    x=r*sinf(t+theta);
    z=r*cosf(t+theta);
}

void XYVector3::RotateZ(float theta)
{
    float r=sqrt(x*x+y*y);
    if(r==0) return;
    float t=acosf(x/r);
    if(y<0) t=2*Pi-t;
    y=r*sinf(t+theta);
    x=r*cosf(t+theta);
}

void XYVector3::Rotate(XYVector3& axis,float theta)
{
    XYVector3 A=axis;
    if(SMALL(A.Length())) return;
    A.Unit();
    XYVector3 Ao=DotProduct(*this,A)*A;
    XYVector3 Aex=*this-Ao;
    float Al=Aex.Length();
    Aex.Unit();
    XYVector3 Aey=CrossProduct(A,Aex);
    (*this)=XYVector3(Ao+Al*cosf(theta)*Aex+Al*sin(theta)*Aey);
}

XYVector3 XYVector3::operator+(XYVector3& r)
{
    return XYVector3(x+r.x,y+r.y,z+r.z);
}

XYVector3 XYVector3::operator-(XYVector3& r)
{
    return XYVector3(x-r.x,y-r.y,z-r.z);
}

XYVector3 XYVector3::operator-()
{
    return XYVector3(-x,-y,-z);
}

bool XYVector3::operator==(XYVector3& r)
{
    return SMALL(r.x-x)&&SMALL(r.y-y)&&SMALL(r.z-z);
}

XYVector3 XYVector3::operator*(float r)
{
    return XYVector3(x*r,y*r,z*r);
}

XYVector3 XYVector3::operator/(float r)
{
    assert(r!=0);
    return XYVector3(x/r,y/r,z/r);
}

XYVector3 operator*(float l,XYVector3& r)
{
    return XYVector3(r.x*l,r.y*l,r.z*l);
}

XYVector3 operator*(int l,XYVector3& r)
{
    return XYVector3(r.x*l,r.y*l,r.z*l);
}

float DotProduct(XYVector3& l,XYVector3& r)
{
    return l.x*r.x+l.y*r.y+l.z*r.z;
}

void CrossProduct(XYVector3& l,XYVector3& r,XYVector3& res)
{
    res.x=l.y*r.z-l.z*r.y;
    res.y=l.z*r.x-l.x*r.z;
    res.z=l.x*r.y-l.y*r.x;
}

XYVector3 CrossProduct(XYVector3& l,XYVector3& r)
{
    return XYVector3(
        l.y*r.z-l.z*r.y,
        l.z*r.x-l.x*r.z,
        l.x*r.y-l.y*r.x
    );
}

void Unit(XYVector3& v,XYVector3& res)
{
    res=v/v.Length();
}

XYVector3 Unit(XYVector3& v)
{
    if(SMALL(v.Length()))
        return VEC_NULL;
    return (v/v.Length());
}

void Eye(XYVector3& v)
{
    v.x=v.y=v.z=1;
}

//反射系数和光照强度
XYRayI::XYRayI()
{

}

XYRayI::XYRayI(float _r,float _g,float _b)
    :r(_r),g(_g),b(_b)
{

}

XYRayI XYRayI::operator+(XYRayI& I)
{
    return XYRayI(r+I.r,g+I.g,b+I.b);
}

XYRayI XYRayI::operator*(XYRayK& K)
{
    return XYRayI(r*K.r,g*K.g,b*K.b);
}

XYRayI XYRayI::operator*(float d)
{
    return XYRayI(r*d,g*d,b*d);
}

void XYRayI::Adjust()
{
    if(r>1) r=1.0f;
    if(g>1) g=1.0f;
    if(b>1) b=1.0f;
}

XYRayK::XYRayK()
{

}

XYRayK::XYRayK(COLORREF col)
    :r(GetRValue(col)/255.0f),g(GetGValue(col)/255.0f),b(GetBValue(col)/255.0f)
{

}

XYRayK::XYRayK(float _r,float _g,float _b)
    :r(_r),g(_g),b(_b)
{

}

//Normal distribution
float Gauss2(float u,float v,float sigma)
{
    return exp(-(u*u+v*v)/2/sigma/sigma)/2/Pi/sigma/sigma;
}
