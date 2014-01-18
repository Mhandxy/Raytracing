#include "XY.h"

XYObject::XYObject()
    :o(VEC_ORIGIN)
{
}

XYObject::XYObject(XYVector3& _o)
    :o(_o)
{
}

XYObject::~XYObject()
{
}

XYVector3 XYObject::GetOrigin()
{
    return o;
}

void XYObject::Illumination(XYVector3& ro,XYVector3& dir,float& t,
                            XYVector3& P,XYVector3& R,XYRayK& Ks,
                            XYVector3& T,XYRayK& Kt,XYRayI& Il)
{
    if(dir==VEC_NULL) return;
    P=ro+dir*t;
    XYVector3 V=Unit(-dir);
    XYVector3 N=GetNormal(P);
    XYRayK Ka,Kd;
    float rn;
    Reflex(P,V,R);
    bool TIR=Refract(P,V,T);
    GetMaterial(P,V,Ka,Kd,Ks,Kt,rn,TIR);
    
    Il=XYGetEnvCol()*Ka;

    int lc=XYGetLight()->GetCount();
    for(int i=0;i<lc;++i)
    {
        XYVector3 L=Unit((*XYGetLight())[i]->GetDir(P));
        XYVector3 H=Unit(V+L);
        float d=DotProduct(L,N);
        float s=pow(DotProduct(H,N),rn);
        //ÒõÓ°
        (*XYGetLight())[i]->GetShadow(P,d,s);
        if(rn<0)
            s=0;
        XYRayI Ip=(*XYGetLight())[i]->GetIll();
        XYRayI tmpI=Ip*Kd*d+Ip*Ks*s;
        tmpI.Adjust();
        Il=Il+tmpI;
    }
}


///////////////////////////////////////////

XYSphere::XYSphere(XYVector3& _o,float _r,XYMaterial _mat)
    :XYObject(_o),r(_r),index(1.3f)
{
    mat=_mat;
}

XYVector3 XYSphere::GetNormal(XYVector3& P)
{
    return Unit(P-o);
}

void XYSphere::Reflex(XYVector3& P,XYVector3& V,XYVector3& R)
{
    XYVector3 N=GetNormal(P);
    R=2*DotProduct(V,N)*N-V;
}

bool XYSphere::Refract(XYVector3& P,XYVector3& V,XYVector3& T)
{
    XYVector3 N=GetNormal(P);
    float phi=acosf(abs(DotProduct(V,N)));
    float theta;
    if(DotProduct(V,N)>0)
    {
        theta=asinf(sinf(phi)/index);
        T=-Unit(N+Unit((V-DotProduct(V,N)*N))*tanf(theta));
    }
    else
    {
        N=-N;
        theta=asinf(sinf(phi)*index);
        if(sinf(phi)<1/index)
            T=-Unit(N+Unit((V-DotProduct(V,N)*N))*tanf(theta));
        else
        {
            T=VEC_NULL;
            return true;
        }
    }
    return false;
}

void XYSphere::GetMaterial( XYVector3& P,XYVector3& V,XYRayK& Ka,XYRayK& Kd,
                            XYRayK& Ks,XYRayK& Kt,float& rn,bool TIR)
{
    if(mat==XY_MAT_SPH_1)
    {
        Ka=Kd=XYRayK(50/255.0f,50/255.0f,50/255.0f);
        Ks=XYRayK(0.25f,0.25f,0.25f);
        Kt=XYRayK(0.95f,0.95f,0.95f);
        rn=1000;
        if(TIR)
        {
            Ks=XYRayK(1.0f,1.0f,1.0f);
            Kt=XYRayK(0.0f,0.0f,0.0f);
        }
    }
    else if(mat==XY_MAT_SPH_2)
    {
        float r,u,v;
        GetNormal(P).SphCoor(r,u,v);
        
        Ks=XYRayK(0.4f,0.4f,0.4f);
        Kt=XYRayK(0.0f,0.0f,0.0f);
        rn=1000;

        if((QZ(r*10*u/Pi)+QZ(r*10*v/Pi))%2==0)
            Kd=XYRayK(255/255.0f,255/255.0f,255/255.0f);
        else
            Kd=XYRayK(0/255.0f,0/255.0f,0/255.0f);
        Ka=Kd;
    }
    else if(mat==XY_MAT_SPH_3)
    {
        float r,u,v;
        Ks=XYRayK(0.3f,0.3f,0.3f);
        Kt=XYRayK(0.0f,0.0f,0.0f);
        rn=500;
        static XYTexture* tex;
        static bool flag=true;
        if(flag)
        {
            tex=new XYTexture(TEXT("T1.bmp"));
            flag=false;
        }
        XYVector3 N=GetNormal(P);
        N.Rotate(XYVector3(-1,1,10),float(Frame)/10.0f*Pi);
        N.SphCoor(r,u,v);
        v+=0.2f;
        Kd=XYRayK(tex->GetColor(v/2.1f/Pi,u/1.05f/Pi,XY_TEX_REL));
        Ka=Kd;
    }
    else
    {
        Ka=Kd=XYRayK(180/255.0f,180/255.0f,180/255.0f);
        Ks=XYRayK(0.4f,0.4f,0.4f);
        Kt=XYRayK(0.0f,0.0f,0.0f);
        rn=1000;
    }
}

void XYSphere::GetAABox(AABox &box)
{
    box.Xmin=o.x-r;
    box.Xmax=o.x+r;
    box.Ymin=o.y-r;
    box.Ymax=o.y+r;
    box.Zmin=o.z-r;
    box.Zmax=o.z+r;
}

bool XYSphere::RayIntersection(XYVector3& ro,XYVector3& dir,float& t)
{
    float Loc_2=DotProduct(o-ro,o-ro);
    float tca=DotProduct(o-ro,dir);
    float thc_2=r*r-Loc_2+tca*tca;
    if(thc_2<0)
        return false;
    t=(tca-sqrt(thc_2));
    if((!SMALL(t))&&t<0)
        return false;
    if(SMALL(t))
        t=(tca+sqrt(thc_2));
    return true;
}
/////////////////////////////

XYPlane::XYPlane(XYVector3& _o,float _w,float _h,XYMaterial _mat,float theta,float phi)
    :XYObject(_o),h(_h),w(_w)
{
    mat=_mat;
    ex=Unit(XYVector3(1.0f,0.0f,0.0f));
    ex.RotateX(theta);
    ex.RotateY(phi);
    ey=Unit(XYVector3(0.0f,1.0f,0.0f));
    ey.RotateX(theta);
    ey.RotateY(phi);
    n=Unit(CrossProduct(ex,ey));
}

XYVector3 XYPlane::GetNormal(XYVector3& P)
{
    return n;
}

void XYPlane::Reflex(XYVector3& P,XYVector3& V,XYVector3& R)
{
    if(DotProduct(V,n)<0)
    {
        R=VEC_NULL;
        return;
    }
    R=Unit(V);
    R.Rotate(n,Pi);
}

bool XYPlane::Refract(XYVector3& P,XYVector3& V,XYVector3& T)
{
    T=VEC_NULL;
    return false;
}

void XYPlane::GetMaterial( XYVector3& P,XYVector3& V,XYRayK& Ka,XYRayK& Kd,
                           XYRayK& Ks,XYRayK& Kt,float& rn,bool TIR)
{
    if(mat==XY_MAT_PLA_1)
    {
        Ks=XYRayK(0.0f,0.0f,0.0f);
        Kt=XYRayK(0.0f,0.0f,0.0f);
        rn=100;
        if((QZ(DotProduct(P-o,ex))+QZ(DotProduct(P-o,ey)))%2==0)
            Kd=XYRayK(200/255.0f,75/255.0f,35/255.0f);
        else
            Kd=XYRayK(225/255.0f,210/255.0f,10/255.0f);
        Ka=Kd;
        if(DotProduct(V,n)<0)
            Kd=XYRayK(0.0f,0.0f,0.0f);
    }
    else if(mat==XY_MAT_PLA_2)
    {
        Ks=XYRayK(0.0f,0.0f,0.0f);
        Kt=XYRayK(0.0f,0.0f,0.0f);
        rn=100;
        static XYTexture* tex;
        static bool flag=true;
        if(flag)
        {
            tex=new XYTexture(TEXT("T2.bmp"));
            flag=false;
        }
        int x=int((DotProduct(P-o,ex)+h/2)/h*3000);
        int y=int((DotProduct(P-o,ey)+w/2)/w*15000);
        Kd=XYRayK(tex->GetColor(float(x),float(y),XY_TEX_ABS));
        Ka=Kd;
        if(DotProduct(V,n)<0)
            Kd=XYRayK(0.0f,0.0f,0.0f);
    }
    else if(mat==XY_MAT_PLA_3)
    {
        Ks=XYRayK(0.0f,0.0f,0.0f);
        Kt=XYRayK(0.0f,0.0f,0.0f);
        rn=100;
        static XYTexture* tex;
        static bool flag=true;
        if(flag)
        {
            tex=new XYTexture(TEXT("T3.bmp"));
            flag=false;
        }
        int x=int((DotProduct(P-o,ex)+h/2)/h*1000);
        int y=int((DotProduct(P-o,ey)+w/2)/w*6000);
        Kd=XYRayK(tex->GetColor(float(y),float(x),XY_TEX_ABS));
        Ka=Kd;
        if(DotProduct(V,n)<0)
            Kd=XYRayK(0.0f,0.0f,0.0f);
    }
    else if(mat==XY_MAT_PLA_4)
    {
        Ks=XYRayK(1.0f,1.0f,1.0f);
        Kt=XYRayK(0.0f,0.0f,0.0f);
        rn=-1;
        Kd=XYRayK(0.0f,0.0f,0.0f);
        Ka=Kd;
    }
    else
    {
        Ks=XYRayK(0.0f,0.0f,0.0f);
        Kt=XYRayK(0.0f,0.0f,0.0f);
        rn=100;
        Kd=XYRayK(220/255.0f,220/255.0f,220/255.0f);
        Ka=Kd;
        if(DotProduct(V,n)<0)
            Kd=XYRayK(0.0f,0.0f,0.0f);
    }
}

void XYPlane::GetAABox(AABox &box)
{
    XYVector3 p1=o+h/2.0f*ex+w/2.0f*ey;
    XYVector3 p2=o+h/2.0f*ex-w/2.0f*ey;
    XYVector3 p3=o-h/2.0f*ex-w/2.0f*ey;
    XYVector3 p4=o-h/2.0f*ex+w/2.0f*ey;
    box.Xmin=min(min(p1.x,p2.x),min(p3.x,p4.x))-0.1f;
    box.Xmax=max(max(p1.x,p2.x),max(p3.x,p4.x))+0.1f;
    box.Ymin=min(min(p1.y,p2.y),min(p3.y,p4.y))-0.1f;
    box.Ymax=max(max(p1.y,p2.y),max(p3.y,p4.y))+0.1f;
    box.Zmin=min(min(p1.z,p2.z),min(p3.z,p4.z))-0.1f;
    box.Zmax=max(max(p1.z,p2.z),max(p3.z,p4.z))+0.1f;
}

bool XYPlane::RayIntersection(XYVector3& ro,XYVector3& dir,float& t)
{
    if(SMALL(DotProduct(dir,n))) return false;
    t=(DotProduct(o-ro,n))/DotProduct(dir,n);
    if(t<0||SMALL(t)) return false;

    XYVector3 P=ro+dir*t;
    if(abs(DotProduct(P-o,ex))>h/2.0f||abs(DotProduct(P-o,ey))>w/2.0f)
        return false;
    return true;
}
//////////////////////////////////////////////////////////
XYTriangle::XYTriangle(XYVector3& _p1,XYVector3& _p2,XYVector3& _p3)
    :XYObject((_p1+_p2+_p3)/3.0f),p1(_p1),p2(_p2),p3(_p3)
{
    n=Unit(CrossProduct(p2-p1,p3-p1));
}

XYVector3 XYTriangle::GetNormal(XYVector3& P)
{
    return n;
}

void XYTriangle::Reflex(XYVector3& P,XYVector3& V,XYVector3& R)
{
    R=Unit(V);
    R.Rotate(n,Pi);
}

bool XYTriangle::Refract(XYVector3& P,XYVector3& V,XYVector3& T)
{
    T=VEC_NULL;
    return false;
}

void XYTriangle::GetMaterial( XYVector3& P,XYVector3& V,XYRayK& Ka,XYRayK& Kd,
                           XYRayK& Ks,XYRayK& Kt,float& rn,bool TIR)
{
    Ks=XYRayK(0.0f,0.0f,0.0f);
    Kt=XYRayK(0.0f,0.0f,0.0f);
    rn=100;
    Kd=XYRayK(0.5f,0.5f,0.5f);
    Ka=Kd;
}

void XYTriangle::GetAABox(AABox &box)
{
    box.Xmin=min(min(p1.x,p2.x),p3.x)-0.1f;
    box.Xmax=max(max(p1.x,p2.x),p3.x)+0.1f;
    box.Ymin=min(min(p1.y,p2.y),p3.y)-0.1f;
    box.Ymax=max(max(p1.y,p2.y),p3.y)+0.1f;
    box.Zmin=min(min(p1.z,p2.z),p3.z)-0.1f;
    box.Zmax=max(max(p1.z,p2.z),p3.z)+0.1f;
}

bool XYTriangle::RayIntersection(XYVector3& ro,XYVector3& dir,float& t)
{
    if(SMALL(DotProduct(dir,n))) return false;
    t=(DotProduct(o-ro,n))/DotProduct(dir,n);
    if(t<0||SMALL(t)) return false;

    XYVector3 P=ro+dir*t;
    float S=CrossProduct(P-p1,P-p2).Length()+CrossProduct(P-p2,P-p3).Length()+CrossProduct(P-p3,P-p1).Length();
    float OS=CrossProduct(p3-p1,p3-p2).Length();
    if(OS+EPSILON<S)
        return false;
    return true;
}
//////////////////////////////////////////////////////////

XYBox::XYBox(XYVector3& _o,float _a,float _b,float _c,XYMaterial _mat,float theta,float phi)
    :XYObject(_o),a(_a),b(_b),c(_c),index(1.3f)
{
    mat=_mat;
    ex=Unit(XYVector3(1.0f,0.0f,0.0f));
    ex.RotateX(theta);
    ex.RotateY(phi);
    ey=Unit(XYVector3(0.0f,1.0f,0.0f));
    ey.RotateX(theta);
    ey.RotateY(phi);
    ez=Unit(CrossProduct(ex,ey));
}

XYVector3 XYBox::GetNormal(XYVector3& P)
{
    if(SMALL(DotProduct(P-o,ex)-a/2))
        return ex;
    if(SMALL(DotProduct(P-o,ex)+a/2))
        return -ex;
    if(SMALL(DotProduct(P-o,ey)-b/2))
        return ey;
    if(SMALL(DotProduct(P-o,ey)+b/2))
        return -ey;
    if(SMALL(DotProduct(P-o,ez)-c/2))
        return ez;
    if(SMALL(DotProduct(P-o,ez)+c/2))
        return -ez;
    return VEC_NULL;
}

void XYBox::Reflex(XYVector3& P,XYVector3& V,XYVector3& R)
{
    XYVector3 N=GetNormal(P);
    R=Unit(V);
    R.Rotate(N,Pi);
}

bool XYBox::Refract(XYVector3& P,XYVector3& V,XYVector3& T)
{
    XYVector3 N=GetNormal(P);
    float phi=acosf(abs(DotProduct(V,N)));
    float theta;
    if(DotProduct(V,N)>0)
    {
        theta=asinf(sinf(phi)/index);
        T=-Unit(N+Unit((V-DotProduct(V,N)*N))*tanf(theta));
    }
    else
    {
        N=-N;
        theta=asinf(sinf(phi)*index);
        if(sinf(phi)<1/index+EPSILON)
            T=-Unit(N+Unit((V-DotProduct(V,N)*N))*tanf(theta));
        else
        {
            T=VEC_NULL;
            return true;
        }
    }
    return false;
}

void XYBox::GetMaterial( XYVector3& P,XYVector3& V,XYRayK& Ka,XYRayK& Kd,
                           XYRayK& Ks,XYRayK& Kt,float& rn,bool TIR)
{
    if(mat==XY_MAT_BOX_1)
    {
        Ka=Kd=XYRayK(100/255.0f,100/255.0f,100/255.0f);
        Ks=XYRayK(0.0f,0.0f,0.0f);
        Kt=XYRayK(0.9f,0.9f,0.9f);
        rn=1000;
        XYVector3 N=GetNormal(P);
        if(TIR)
        {
            Ks=XYRayK(0.9f,0.9f,0.9f);
            Kt=XYRayK(0.0f,0.0f,0.0f);
        }
    }
    else
    {
        Ks=XYRayK(0.5f,0.5f,0.5f);
        Kt=XYRayK(0.0f,0.0f,0.0f);
        rn=1000;
        Kd=XYRayK(100/255.0f,100/255.0f,100/255.0f);
        if(DotProduct(V,GetNormal(P))<0)
            Kd=XYRayK(0.0f,0.0f,0.0f);
        Ka=Kd;
    }
}
void XYBox::GetAABox(AABox &box)
{
    box.Xmin=o.x-a/2.0f;
    box.Xmax=o.x+a/2.0f;
    box.Ymin=o.y-b/2.0f;
    box.Ymax=o.y+b/2.0f;
    box.Zmin=o.z-c/2.0f;
    box.Zmax=o.z+c/2.0f;
}

bool XYBox::RayIntersection(XYVector3& ro,XYVector3& dir,float& t)
{
    float et[6],tmp;
    XYVector3 P;

    for(int i=0;i<2;++i)
    {
        if(SMALL(DotProduct(dir,ex))) 
        {
            et[i]=-1;
            continue;
        }
        tmp=(DotProduct(o-ro,ex)+(i%2==0? (a/2):(-a/2)))/DotProduct(dir,ex);
        if(tmp<EPSILON) 
        {
            et[i]=-1;
            continue;
        }
        P=ro+dir*tmp;
        if(abs(DotProduct(P-o,ey))>b/2.0f||abs(DotProduct(P-o,ez))>c/2.0f)
            et[i]=-1;
        else
            et[i]=tmp;
    }
    for(int i=2;i<4;++i)
    {
        if(SMALL(DotProduct(dir,ey))) 
        {
            et[i]=-1;
            continue;
        }
        tmp=(DotProduct(o-ro,ey)+(i%2==0? (b/2):(-b/2)))/DotProduct(dir,ey);
        if(tmp<EPSILON) 
        {
            et[i]=-1;
            continue;
        }
        P=ro+dir*tmp;
        if(abs(DotProduct(P-o,ex))>a/2.0f||abs(DotProduct(P-o,ez))>c/2.0f)
            et[i]=-1;
        else
            et[i]=tmp;
    }
    for(int i=4;i<6;++i)
    {
        if(SMALL(DotProduct(dir,ez))) 
        {
            et[i]=-1;
            continue;
        }
        tmp=(DotProduct(o-ro,ez)+(i%2==0? (c/2):(-c/2)))/DotProduct(dir,ez);
        if(tmp<EPSILON) 
        {
            et[i]=-1;
            continue;
        }
        P=ro+dir*tmp;
        if(abs(DotProduct(P-o,ex))>a/2.0f||abs(DotProduct(P-o,ey))>b/2.0f)
            et[i]=-1;
        else
            et[i]=tmp;
    }
    t=-1;
    for(int i=0;i<6;++i)
    {
        if(et[i]==-1)
            continue;
        if(t==-1)
            t=et[i];
        else if(t>et[i])
            t=et[i];
    }
    if(t==-1)
        return false;
    return true;
}