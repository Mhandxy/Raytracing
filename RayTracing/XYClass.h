#pragma once
#define _CRT_SECURE_NO_WARNINGS
#include <Windows.h>
#include <string>
#include <math.h>
#include <stdlib.h>
#include <vector>
#include <list>
#include <algorithm>
#include <assert.h>
using namespace std;
#define private public
#define protected public

//常用宏与对象
#define COL_BLACK   RGB(0,0,0)
#define COL_WHITE   RGB(255,255,255)
#define ILL_BLACK   XYRayI(0.0f,0.0f,0.0f)
#define ILL_WHITE   XYRayI(1.0f,1.0f,1.0f)
#define VEC_ORIGIN  XYVector3(0,0,0)
#define VEC_NULL    XYVector3(0,0,0)

//lighttype
typedef int LightType;
#define XY_LT_POINT     1
#define XY_LT_PARALLEL  2
//material
typedef int XYMaterial;
#define XY_MAT_SPH_0    10
#define XY_MAT_SPH_1    11
#define XY_MAT_SPH_2    12
#define XY_MAT_SPH_3    13
#define XY_MAT_SPH_4    14

#define XY_MAT_PLA_0    20
#define XY_MAT_PLA_1    21
#define XY_MAT_PLA_2    22
#define XY_MAT_PLA_3    23
#define XY_MAT_PLA_4    24

#define XY_MAT_BOX_0    30
#define XY_MAT_BOX_1    31
#define XY_MAT_BOX_2    32
#define XY_MAT_BOX_3    33

#define SPH 0
//向量
class XYVector3
{
public:
    XYVector3();
    XYVector3(float _x,float _y,float _z);
    XYVector3(float r,float theta,float phi,int unuse);
    //attribute
    float Length();
    void SphCoor(float& r,float& theta,float& phi);
    //transfer
    void Unit();
    void RotateX(float theta);
    void RotateY(float theta);
    void RotateZ(float theta);
    void Rotate(XYVector3& axis,float theta);
    //operator
    XYVector3 operator+(XYVector3& r);
    XYVector3 operator-(XYVector3& r);
    XYVector3 operator-();
    bool operator==(XYVector3& r);
    XYVector3 operator*(float r);
    XYVector3 operator/(float r);
    friend XYVector3 operator*(float l,XYVector3& r);
    friend XYVector3 operator*(int l,XYVector3& r);

    friend void Unit(XYVector3& v,XYVector3& res);
    friend XYVector3 Unit(XYVector3& v);
    friend float DotProduct(XYVector3& l,XYVector3& r);
    friend XYVector3 CrossProduct(XYVector3& l,XYVector3& r);
    friend void CrossProduct(XYVector3& l,XYVector3& r,XYVector3& res);
    friend void Eye(XYVector3& v);
public:
    float x;
    float y;
    float z;
};

//反射系数
class XYRayK
{
public:
    float r;
    float g;
    float b;
    XYRayK();
    XYRayK(COLORREF col);
    XYRayK(float _r,float _g,float _b);
};

//Illumination
class XYRayI
{
public:
    float r;
    float g;
    float b;
    XYRayI();
    XYRayI(float _r,float _g,float _b);
    XYRayI operator+(XYRayI& I);
    XYRayI operator*(XYRayK& K);
    XYRayI operator*(float d);
    void Adjust();
};

//光源
class XYLightSource
{
public:
    XYLightSource(LightType _type=XY_LT_POINT,
            XYVector3& _para=XYVector3(0,0,10),
            XYRayI& _ill=ILL_WHITE);
    XYRayI GetIll();
    XYVector3 GetDir(XYVector3& P);
    void GetShadow(XYVector3& P,float& d,float& s);
private:
    XYVector3 para;
    XYRayI ill;
    LightType type;
};

class XYLight
{
public:
    ~XYLight();
    void AddLight(XYLightSource* ls);
    XYLightSource* operator[](int i);
    int GetCount();
private:
    vector<XYLightSource*> light;
};

class AABox
{
public:
    float Xmin;
    float Xmax;
    float Ymin;
    float Ymax;
    float Zmin;
    float Zmax;
    bool RayIntersection(XYVector3& ro,XYVector3& dir);
    friend bool AABoxIntersect(AABox &box1,AABox &box2);
};

//物体与场景
class XYObject
{
public:
    XYObject();
    XYObject(XYVector3& _o);
    virtual ~XYObject();
    XYVector3 GetOrigin();
public:
    virtual bool RayIntersection(XYVector3& ro,XYVector3& dir,float& t)=0;
    virtual void Reflex(XYVector3& P,XYVector3& V,XYVector3& R)=0;
    virtual bool Refract(XYVector3& P,XYVector3& V,XYVector3& T)=0;
    virtual XYVector3 GetNormal(XYVector3& P)=0;
    virtual void GetAABox(AABox &box)=0;
    virtual void GetMaterial( XYVector3& P,XYVector3& V,XYRayK& Ka,XYRayK& Kd,
                              XYRayK& Ks,XYRayK& Kt,float& rn,bool TIR)=0;
    virtual void Illumination( XYVector3& ro,XYVector3& dir,float& t,
                               XYVector3& P,XYVector3& R,XYRayK& Ks,
                               XYVector3& T,XYRayK& Kt,XYRayI& Il);
protected:
    XYVector3 o;
    XYMaterial mat;
};

#include "SimpleObject.h"
using namespace SimpleOBJ;

class TNode
{
public:
    AABox box;
    list<XYObject*> obj;
    TNode* lchild;
    TNode* rchild;
};

class BSPTree
{
private:
    TNode* root; 
public:
    BSPTree();
    ~BSPTree();
    void Init(AABox &sbox);
    void Destory(TNode* pnode);
    void InsertObject(XYObject* pobj);
    void InsertObject(TNode* pnode,int depth,XYObject* pobj);
    XYObject* Intersection(TNode* pnode,XYVector3& ro,XYVector3& dir,float &tmin);
    XYObject* Intersection(XYVector3& ro,XYVector3& dir,float &tmin);
    void SaveToObj(char* filename);
    void SaveToObj(TNode*pnode,char* filename);
};

class XYScene
{
public:
    XYScene(float ran=10.0f);
    ~XYScene();
    bool RayIntersection(XYVector3& ro,XYVector3& dir,
        XYVector3& P,XYVector3& R,XYRayK& Ks,
        XYVector3& T,XYRayK& Kt,XYRayI& Il,float& tmin);
    bool RayIntersection(XYVector3& ro,XYVector3& dir);
    bool RayIntersection(XYVector3& ro,XYVector3& dir,float &tmin);
    bool RayIntersection(XYVector3& ro,XYVector3& dir,float &tmin,XYObject* &ptrobj);
    void AddObj(XYObject*);
    void AddMeshObj(CSimpleObject* meshobj,XYVector3& ori=VEC_ORIGIN,float size=1.0f);
private:
    list<XYObject*> obj;
    float range;
    BSPTree bsptree;
};

class XYPlane:public XYObject
{
public:
    XYPlane(XYVector3& _o,float _w,float _h,XYMaterial mat=XY_MAT_PLA_0,float theta=0.0f,float phi=0.0f);
public:
    XYVector3 GetNormal(XYVector3& P);
    void Reflex(XYVector3& P,XYVector3& V,XYVector3& R);
    bool Refract(XYVector3& P,XYVector3& V,XYVector3& T);
    void GetMaterial( XYVector3& P,XYVector3& V,XYRayK& Ka,XYRayK& Kd,
                      XYRayK& Ks,XYRayK& Kt,float& rn,bool TIR);
    void GetAABox(AABox &box);
    bool RayIntersection(XYVector3& ro,XYVector3& dir,float& t);
private:
    float w;
    float h;
    XYVector3 ex;
    XYVector3 ey;
    XYVector3 n;
};

class XYTriangle:public XYObject
{
public:
    XYTriangle(XYVector3& _p1,XYVector3& _p2,XYVector3& _p3);
public:
    XYVector3 GetNormal(XYVector3& P);
    void Reflex(XYVector3& P,XYVector3& V,XYVector3& R);
    bool Refract(XYVector3& P,XYVector3& V,XYVector3& T);
    void GetMaterial( XYVector3& P,XYVector3& V,XYRayK& Ka,XYRayK& Kd,
                      XYRayK& Ks,XYRayK& Kt,float& rn,bool TIR);
    void GetAABox(AABox &box);
    bool RayIntersection(XYVector3& ro,XYVector3& dir,float& t);
private:
    XYVector3 p1;
    XYVector3 p2;
    XYVector3 p3;
    XYVector3 n;
};

class XYBox:public XYObject
{
public:
    XYBox(XYVector3& _o,float _a,float _b,float _c,XYMaterial _mat=XY_MAT_BOX_0,float theta=0.0f,float phi=0.0f);
public:
    XYVector3 GetNormal(XYVector3& P);
    void Reflex(XYVector3& P,XYVector3& V,XYVector3& R);
    bool Refract(XYVector3& P,XYVector3& V,XYVector3& T);
    void GetMaterial( XYVector3& P,XYVector3& V,XYRayK& Ka,XYRayK& Kd,
                      XYRayK& Ks,XYRayK& Kt,float& rn,bool TIR);
    void GetAABox(AABox &box);
    bool RayIntersection(XYVector3& ro,XYVector3& dir,float& t);
private:
    float a;
    float b;
    float c;
    XYVector3 ex;
    XYVector3 ey;
    XYVector3 ez;
    float index;
};

class XYSphere:public XYObject
{
public:
    XYSphere(XYVector3& _o,float _r,XYMaterial _mat=XY_MAT_SPH_0);
public:
    XYVector3 GetNormal(XYVector3& P);
    void Reflex(XYVector3& P,XYVector3& V,XYVector3& R);
    bool Refract(XYVector3& P,XYVector3& V,XYVector3& T);
    void GetMaterial( XYVector3& P,XYVector3& V,XYRayK& Ka,XYRayK& Kd,
                      XYRayK& Ks,XYRayK& Kt,float& rn,bool TIR);
    void GetAABox(AABox &box);
    bool RayIntersection(XYVector3& ro,XYVector3& dir,float& t);
public:
    float r;
    float index;
};

#define XY_TEX_REL       1
#define XY_TEX_ABS       2
class XYTexture
{
public:
    XYTexture(LPWSTR lpFileName);
    ~XYTexture();
    COLORREF GetColor(float i,float j,int PARA=XY_TEX_REL);
private:
    HBITMAP hbitmap;
    LPWSTR FileName;
    BYTE* pBits;
    int bwidth;
    int bheight;
    int bit;
};

//缓存
class XYScreenBuffer
{
public:
    XYScreenBuffer(HDC _hdc);
    ~XYScreenBuffer();
    BYTE* GetBuffer();
    HDC GetMemDC();
    HDC GetHDC();
    void SwapBuffer();
private:
    HBITMAP membm;
    HDC hmemdc;
    HDC hdc;
    BYTE* buffer;
};

//相机
class XYCamera
{
public:
    XYCamera(int w,int h,XYVector3& _o=XYVector3(10,0,5),XYVector3& dir=VEC_ORIGIN);
    ~XYCamera();
    void Init();
    XYVector3 GetOrigin();
    void Lookat(XYVector3& _o,XYVector3& dir,XYVector3& up=XYVector3(0,0,1));
    void Image(XYScene* scene,XYScreenBuffer* buffer);
private:
    COLORREF RayTracing(XYScene* scene,XYVector3& dir,float& tmin);
    void RayTracing(XYScene* scene,XYVector3& ro,XYVector3& dir,float weight,XYRayI& I,float& tmin);
private:
    int pixelx;
    int pixely;
    float width;
    float height;
    float* ZBuffer;
    float F;
    float L;
    XYVector3 o;
    XYVector3 ex;
    XYVector3 ey;
    XYVector3 no;
    XYVector3 d;
};
