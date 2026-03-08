
#include "global.h"
#include "ecranConstantesPosition.h"
#include "MyBoutonSerie.h"


bool CMyBoutonBase::isPressed() {
    if (!touch.touched()) return false;
    

    TS_Point p = touch.getPoint();
    int ty = map(p.y, TS_MINY, TS_MAXY, 0, RESOLUTION_Y);
    int tx = map(p.x, TS_MINX, TS_MAXX, 0, RESOLUTION_X);

    bool inside = (ty >= muiPosY && ty <= muiPosY + muiHight) && (tx >= muiPosX && tx <= muiPosX + muiWidth);
    
    if (inside) {
        waitRelease();
        return true;
    }
    return false;;
}

void CMyBoutonBase::waitRelease() {
    while (touch.touched()) delay(10);
}

