#pragma once
#include <vector>
#include <list>
#include "../structures/camera.h"
#include "save_png.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "buffer_render.h"

#define SCREENHEIGHT 200
#define MAXPTS 10

#define putdot(x,y) if ((y >= 0) && (y < SCREENHEIGHT)) dots[y][counters[y]++] = x;

void fillpoly(u8* buffer, s16* datas, int n, u8 c) {
    static int dots[SCREENHEIGHT][MAXPTS];
    static int counters[SCREENHEIGHT];
    s16 x1, y1, x2, y2;
    int i, j, k, dir = -2;
    double step, curx;

    if (n <= 2) {
        switch (n) {
        case 0:
            return;
        case 1:
            pixel(buffer, datas[0], datas[1], c);
            return;
        case 2:
            line(buffer, datas[0], datas[1], datas[2], datas[3], c);
            return;
        }
    }

    // Reinit array counters

    for (i = 0; i < SCREENHEIGHT; i++) {
        counters[i] = 0;
    }

    // Drawing lines

    x2 = datas[n * 2 - 2];
    y2 = datas[n * 2 - 1];

    for (i = 0; i < n; i++) {
        x1 = x2;
        y1 = y2;
        x2 = datas[i * 2];
        y2 = datas[i * 2 + 1];

        //  line(x1, y1, x2, y2, c);
        //  continue;

        if (y1 == y2) {
            //      printf("Horizontal line. x1: %i, y1: %i, x2: %i, y2: %i\n", x1, y1, x2, y2);
            if (!dir)
                continue;
            putdot(x1, y1);
            dir = 0;
            continue;
        }

        step = (double)(x2 - x1) / (y2 - y1);

        //  printf("x1: %i, y1 = %i, x2 = %i, y2 = %i, step: %f\n", x1, y1, x2, y2, step);

        curx = x1;

        if (y1 < y2) {
            for (j = y1; j < y2; j++, curx += step) {
                //    printf("j = %i, curx = %f\n", j, curx);
                putdot((int)(curx + 0.5), j);
            }
            if (dir == -1) {
                //    printf("Adding extra (%i, %i)\n", x1, y1);
                putdot(x1, y1);
            }
            dir = 1;
        }
        else {
            for (j = y1; j > y2; j--, curx -= step) {
                //    printf("j = %i, curx = %f\n", j, curx);
                putdot((int)(curx + 0.5), j);
            }
            if (dir == 1) {
                //    printf("Adding extra (%i, %i)\n", x1, y1);
                putdot(x1, y1);
            }
            dir = -1;
        }
    }

    x1 = x2;
    y1 = y2;
    x2 = datas[0];
    y2 = datas[1];

    if (((y1 < y2) && (dir == -1)) || ((y1 > y2) && (dir == 1)) || ((y1 == y2) && (dir == 0))) {
        //  printf("Adding final extra (%i, %i)\n", x1, y1);
        putdot(x1, y1);
    }

    // NOTE: all counters should be even now. If not, this is a bad (C) thing :-P

    // Sorting datas

    for (i = 0; i < SCREENHEIGHT; i++) {
        // Very bad sorting... but arrays are very small (0, 2 or 4), so it's no quite use...
        for (j = 0; j < (counters[i] - 1); j++) {
            for (k = 0; k < (counters[i] - 1); k++) {
                if (dots[i][k] > dots[i][k + 1])
                    swap(&dots[i][k], &dots[i][k + 1]);
            }
        }
    }

    // Drawing.

    for (i = 0; i < SCREENHEIGHT; i++) {
        if (counters[i]) {
            //      printf("%i dots on line %i\n", counters[i], i);
            for (j = 0; j < counters[i] - 1; j += 2) {
                //    printf("Drawing line (%i, %i)-%i\n", dots[i][j], dots[i][j + 1], i);
                hline(buffer, dots[i][j], dots[i][j + 1], i, c);
#ifdef DEBUGGING_POLYS
                if ((!dots[i][j]) || !(dots[i][j + 1])) {
                    printf("fillpoly: BLARGH!\n");
                    assert(0);
                }
#endif
            }
        }
    }
}


void renderV1Mask(const std::vector<cameraOverlayPolygon>* srcPolys, const char* outPng) {
    //std::list<p2t::Triangle*> map;

    u8* mask = new u8[320 * 200];
    memset(mask, 0, 320 * 200);

    for (auto& srcPoly : *srcPolys) {        
        auto datas = new s16[srcPoly.size() * 2];
        for (int i = 0; i < srcPoly.size(); i++) {
            datas[i * 2] = srcPoly[i].x;
            datas[i * 2 + 1] = srcPoly[i].y;
        }
        fillpoly(mask, datas, srcPoly.size(), 255);
    }

    savePng(outPng, 320, 200, mask, PNG_COLOR_TYPE_GRAY);
    delete[] mask;
}


//glEnable(GL_STENCIL_TEST);
//
//Then you first draw your masking quad only in the stencil buffer :
//
////begin mask
//glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
//glStencilFunc(GL_NEVER, 0, 0);
//glStencilOp(GL_INCR, GL_KEEP, GL_KEEP);
//glClear(GL_STENCIL_BUFFER_BIT);
////draw your mask, a quad in your case
//
//Then you draw the masked part :
//
//////masked fill part
//glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
//glStencilFunc(GL_NOTEQUAL, 0, 1);
//glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
//// draw the masked element, the text in your case
//
//Disable stencil testing again
//
//glDisable(GL_STENCIL_TEST);

