#include "XY.h"
#define MAXOBJ 20
#define MAXDEPTH 20
bool AABoxIntersect(AABox &box1,AABox &box2)
{
    if( box1.Xmax>box2.Xmin&&box2.Xmax>box1.Xmin&&
        box1.Ymax>box2.Ymin&&box2.Ymax>box1.Ymin&&
        box1.Zmax>box2.Zmin&&box2.Zmax>box1.Zmin)
        return true;
    else
        return false;       
}

bool AABox::RayIntersection(XYVector3& ro,XYVector3& dir)
{
    float tx,ty,tz;
    if(!SMALL(dir.x))
        tx=min((Xmin-ro.x)/dir.x,(Xmax-ro.x)/dir.x);
    else
        tx=-1e7;
    if(!SMALL(dir.y))
        ty=min((Ymin-ro.y)/dir.y,(Ymax-ro.y)/dir.y);
    else
        ty=-1e7;
    if(!SMALL(dir.z))
        tz=min((Zmin-ro.z)/dir.z,(Zmax-ro.z)/dir.z);
    else
        tz=-1e7;
    if(tx>=ty&&tx>=tz)
    {
        XYVector3 p=ro+tx*dir;
        if(p.y<=Ymax&&p.y>=Ymin&&p.z<=Zmax&&p.z>=Zmin)
            return true;
        else
            return false;
    }
    else if(ty>=tx&&ty>=tz)
    {
        XYVector3 p=ro+ty*dir;
        if(p.x<=Xmax&&p.x>=Xmin&&p.z<=Zmax&&p.z>=Zmin)
            return true;
        else
            return false;
    }
    else //if(tz>=tx&&tz>=ty)
    {
        XYVector3 p=ro+tz*dir;
        if(p.y<=Ymax&&p.y>=Ymin&&p.x<=Xmax&&p.x>=Xmin)
            return true;
        else
            return false;
    }
}

BSPTree::BSPTree()
{
}

BSPTree::~BSPTree()
{
    Destory(root);
}

void BSPTree::Destory(TNode* pnode)
{
    if(pnode==NULL) return;
    Destory(pnode->rchild);
    Destory(pnode->lchild);
    SAFE_RELEASE(pnode);
}

void BSPTree::Init(AABox &sbox)
{
    root=new TNode();
    root->box=sbox;
}

void BSPTree::InsertObject(XYObject* pobj)
{
    InsertObject(root,1,pobj);
}
void BSPTree::InsertObject(TNode* pnode,int depth,XYObject* pobj)
{
    if(pnode->lchild==NULL&&pnode->rchild==NULL)
    {
        if(pnode->obj.size()<MAXOBJ||depth>=MAXDEPTH)
        {
            AABox box;
            pobj->GetAABox(box);
            if(AABoxIntersect(box,pnode->box))
                pnode->obj.push_back(pobj);
        }
        else
        {
            TNode *lc,*rc;
            lc=new TNode();
            rc=new TNode();
            float X,Y,Z;
            X=pnode->box.Xmax-pnode->box.Xmin;
            Y=pnode->box.Ymax-pnode->box.Ymin;
            Z=pnode->box.Zmax-pnode->box.Zmin;
            if(X>=Y&&X>=Z)
            {
                float mid=(pnode->box.Xmax+pnode->box.Xmin)/2.0f;
                lc->box=pnode->box;
                lc->box.Xmax=mid;
                rc->box=pnode->box;
                rc->box.Xmin=mid;
            }
            else if(Y>=Z&&Y>=X)
            {
                float mid=(pnode->box.Ymax+pnode->box.Ymin)/2.0f;
                lc->box=pnode->box;
                lc->box.Ymax=mid;
                rc->box=pnode->box;
                rc->box.Ymin=mid;
            }
            else
            {
                float mid=(pnode->box.Zmax+pnode->box.Zmin)/2.0f;
                lc->box=pnode->box;
                lc->box.Zmax=mid;
                rc->box=pnode->box;
                rc->box.Zmin=mid;
            }
            for(auto iter=pnode->obj.begin();iter!=pnode->obj.end();++iter)
            {
                AABox box;
                (*iter)->GetAABox(box);
                if(AABoxIntersect(box,lc->box))
                    lc->obj.push_back(*iter);
                if(AABoxIntersect(box,rc->box))
                    rc->obj.push_back(*iter);
            }
            pnode->obj.clear();
            pnode->lchild=lc;
            pnode->rchild=rc;
            InsertObject(pnode->lchild,depth+1,pobj);
            InsertObject(pnode->rchild,depth+1,pobj);
        }
    }
    else
    {
        InsertObject(pnode->lchild,depth+1,pobj);
        InsertObject(pnode->rchild,depth+1,pobj);
    }
}

XYObject* BSPTree::Intersection(XYVector3& ro,XYVector3& dir,float &tmin)
{
    return Intersection(root,ro,dir,tmin);
}
XYObject* BSPTree::Intersection(TNode* pnode,XYVector3& ro,XYVector3& dir,float &tmin)
{
    tmin=-1;
    if(!pnode->box.RayIntersection(ro,dir))
        return NULL;
    XYObject* ptrobj=NULL;
    if(pnode->lchild==NULL&&pnode->rchild==NULL)
    {
        float t;
        for(auto iter=pnode->obj.begin();iter!=pnode->obj.end();++iter)
        {
            if((*iter)->RayIntersection(ro,dir,t))
            {
                if(tmin==-1)
                {
                    tmin=t;
                    ptrobj=*iter;
                }
                else
                {
                    if(t<tmin)
                    {
                        tmin=t;
                        ptrobj=*iter;
                    }
                }
            }
        }
        return ptrobj;
    }
    else
    {
        float lt,rt;
        XYObject* lobj=Intersection(pnode->lchild,ro,dir,lt);
        XYObject* robj=Intersection(pnode->rchild,ro,dir,rt);
        if(lobj==NULL&&robj==NULL)
            return NULL;
        if(lobj!=NULL&&robj==NULL)
        {
            tmin=lt;
            return lobj;
        }
        if(lobj==NULL&&robj!=NULL)
        {
            tmin=rt;
            return robj;
        }
        if(lt<rt)
        {
            tmin=lt;
            return lobj;
        }
        else
        {
            tmin=rt;
            return robj;
        }
    }
    return NULL;
}
void BSPTree::SaveToObj(char* filename)
{
    FILE* fp=fopen(filename,"w");
    fclose(fp);
    SaveToObj(root,filename);
}
void BSPTree::SaveToObj(TNode* pnode,char* filename)
{
    static int vnum=1;
    FILE* fp=fopen(filename,"a+");
    fprintf(fp,"v %f %f %f\n",pnode->box.Xmax,pnode->box.Ymax,pnode->box.Zmax);
    fprintf(fp,"v %f %f %f\n",pnode->box.Xmax,pnode->box.Ymax,pnode->box.Zmin);
    fprintf(fp,"v %f %f %f\n",pnode->box.Xmax,pnode->box.Ymin,pnode->box.Zmax);
    fprintf(fp,"v %f %f %f\n",pnode->box.Xmax,pnode->box.Ymin,pnode->box.Zmin);
    fprintf(fp,"v %f %f %f\n",pnode->box.Xmin,pnode->box.Ymax,pnode->box.Zmax);
    fprintf(fp,"v %f %f %f\n",pnode->box.Xmin,pnode->box.Ymax,pnode->box.Zmin);
    fprintf(fp,"v %f %f %f\n",pnode->box.Xmin,pnode->box.Ymin,pnode->box.Zmax);
    fprintf(fp,"v %f %f %f\n",pnode->box.Xmin,pnode->box.Ymin,pnode->box.Zmin);
    
    fprintf(fp,"f %d %d %d %d\n",vnum,vnum+1,vnum+3,vnum+2);
    fprintf(fp,"f %d %d %d %d\n",vnum,vnum+1,vnum+5,vnum+4);
    fprintf(fp,"f %d %d %d %d\n",vnum,vnum+4,vnum+6,vnum+2);

    fprintf(fp,"f %d %d %d %d\n",vnum+7,vnum+3,vnum+1,vnum+5);
    fprintf(fp,"f %d %d %d %d\n",vnum+7,vnum+3,vnum+2,vnum+6);
    fprintf(fp,"f %d %d %d %d\n",vnum+7,vnum+6,vnum+4,vnum+5);
    vnum+=8;
    fclose(fp);
    if(pnode->lchild!=NULL)
    {
        SaveToObj(pnode->lchild,filename);
        SaveToObj(pnode->rchild,filename);
    }
}