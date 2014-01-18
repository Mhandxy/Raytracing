#include "XYClass.h"
#define EPSILON     0.0001f
#define MIN(x,y)    (((x)<(y))? (x):(y))
#define MAX(x,y)    (((x)>(y))? (x):(y))
#define QZ(x)       (int((x)<0? ((x)-1):(x)))
#define Pi          3.1415926536f
#define SMALL(x) (abs(x)<EPSILON? 1:0)
#define SAFE_RELEASE(x); if((x)!=NULL) delete (x);

//֡
extern int Frame;

//Interface
void XYInit(HDC _hdc);
void XYRender();
void XYRelease();
void XYLookAt();

XYLight* XYGetLight();
XYRayI XYGetEnvCol();
XYScene* XYGetScene();

//��Ⱦģʽ
typedef int XYMODE;
#define XY_MODE_AA       1
#define XY_MODE_DOF      2
#define XY_MODE_SHA      3

#define XY_AA_CLOSE     10
#define XY_AA_OPEN      11
#define XY_DOF_CLOSE    20
#define XY_DOF_OPEN     21
#define XY_SHA_CLOSE    30
#define XY_SHA_HARD     31
#define XY_SHA_SOFT     32

#include <time.h>
void set_time_base(void);
double get_time_in_seconds(void);

XYMODE XYGetMode(XYMODE i);
XYMODE XYSetMode(XYMODE i);

//other function
BOOL SaveBitmapToFile(HBITMAP hBitmap, LPSTR lpFileName);

float Gauss2(float u,float v,float sigma);