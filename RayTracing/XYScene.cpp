#include "XY.h"
XYScene::XYScene(float ran)
    :range(ran)
{
    AABox sbox={-ran,ran,-ran,ran,-ran,ran};
    bsptree.Init(sbox);
}

XYScene::~XYScene()
{
    for(auto iter=obj.begin();iter!=obj.end();++iter)
    {
        SAFE_RELEASE(*iter);
    }
}

void XYScene::AddObj(XYObject* o)
{
    if(o==NULL) return;
    obj.push_back(o);
    bsptree.InsertObject(o);
}

void XYScene::AddMeshObj(CSimpleObject* meshobj,XYVector3& ori,float size)
{
    XYVector3 oo((meshobj->xmax+meshobj->xmin)/2.0f,(meshobj->ymax+meshobj->ymin)/2.0f,(meshobj->zmax+meshobj->zmin)/2.0f);
    float max=max(max(meshobj->xmax-meshobj->xmin,meshobj->ymax-meshobj->ymin),meshobj->zmax-meshobj->zmin);
    for(int i=0;i<meshobj->m_nTriangles;++i)
    {
        Vec3f v0=meshobj->m_pVertexList[(meshobj->m_pTriangleList+i)->_p[0]];
        Vec3f v1=meshobj->m_pVertexList[(meshobj->m_pTriangleList+i)->_p[1]];
        Vec3f v2=meshobj->m_pVertexList[(meshobj->m_pTriangleList+i)->_p[2]];
        XYObject* o=new XYTriangle(ori+size/max*(XYVector3(v0.x,v0.y,v0.z)-oo),ori+size/max*(XYVector3(v1.x,v1.y,v1.z)-oo),ori+size/max*(XYVector3(v2.x,v2.y,v2.z)-oo));
        obj.push_back(o);
        bsptree.InsertObject(o);
    }
}

bool XYScene::RayIntersection(XYVector3& ro,XYVector3& dir,
        XYVector3& P,XYVector3& R,XYRayK& Ks,
        XYVector3& T,XYRayK& Kt,XYRayI& Il,float& tmin)
{
    Ks=Kt=XYRayK(0.0f,0.0f,0.0f);
    XYObject* ptrobj=NULL;
    XYScene::RayIntersection(ro,dir,tmin,ptrobj);
    if(ptrobj==NULL) return false;
    //求局部光照 折射 反射
    ptrobj->Illumination(ro,dir,tmin,P,R,Ks,T,Kt,Il);
    return true;
}

bool XYScene::RayIntersection(XYVector3& ro,XYVector3& dir)
{
    float tmin;
    XYObject* p;
    return RayIntersection(ro,dir,tmin,p);
}

bool XYScene::RayIntersection(XYVector3& ro,XYVector3& dir,float& tmin)
{
    XYObject* p;
    return RayIntersection(ro,dir,tmin,p);
}

bool XYScene::RayIntersection(XYVector3& ro,XYVector3& dir,float &tmin,XYObject* &ptrobj)
{
    if(obj.size()==0) return false;
    ptrobj=bsptree.Intersection(ro,dir,tmin);
    if(ptrobj==NULL) return false;
    return true;
}