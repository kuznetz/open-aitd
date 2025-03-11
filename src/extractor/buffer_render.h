#pragma once
#include <math.h>
#include <stdlib.h>
#include "structs/int_types.h"

void swap(int* a, int* b)
{
    int temp;
    temp = *a;
    *a = *b;
    *b = temp;
}

#define MIN(a,b) (a<b?a:b)
#define MAX(a,b) (a>b?a:b)
#define SWAP(a,b) (swap(&a,&b))

void pixel(u8* buffer, int x, int y, unsigned char c)
{
    if (x >= 0 && x < 320 && y >= 0 && y < 200)
    {
        *(buffer + y * 320 + x) = c;
    }
}

void hline(u8* buffer, int x1, int x2, int y, unsigned char c)
{
    int i;

    for (i = x1; i < (x2 + 1); i++)
    {
        pixel(buffer, i, y-1, c);
        pixel(buffer, i, y, c);
    }
}

void vline(u8* buffer, int x, int y1, int y2, unsigned char c)
{
    int i;

    for (i = y1; i < (y2 + 1); i++)
    {
        pixel(buffer, x, i, c);
    }
}

void bsubline_1(u8* buffer, int x1, int y1, int x2, int y2, unsigned char c) {
    int x, y, ddx, ddy, e;
    ddx = abs(x2 - x1);
    ddy = abs(y2 - y1) << 1;
    e = ddx - ddy;
    ddx <<= 1;

    if (x1 > x2) {
        SWAP(x1, x2);
        SWAP(y1, y2);
    }

    for (x = x1, y = y1; x <= x2; x++) {
        pixel(buffer, x, y, c);
        if (e < 0) {
            y++;
            e += ddx - ddy;
        }
        else {
            e -= ddy;
        }
    }
}

void bsubline_2(u8* buffer, int x1, int y1, int x2, int y2, unsigned char c) {
    int x, y, ddx, ddy, e;
    ddx = abs(x2 - x1) << 1;
    ddy = abs(y2 - y1);
    e = ddy - ddx;
    ddy <<= 1;

    if (y1 > y2) {
        SWAP(x1, x2);
        SWAP(y1, y2);
    }

    for (y = y1, x = x1; y <= y2; y++) {
        pixel(buffer, x, y, c);
        if (e < 0) {
            x++;
            e += ddy - ddx;
        }
        else {
            e -= ddx;
        }
    }
}

void bsubline_3(u8* buffer, int x1, int y1, int x2, int y2, unsigned char c) {
    int x, y, ddx, ddy, e;
    ddx = abs(x1 - x2) << 1;
    ddy = abs(y2 - y1);
    e = ddy - ddx;
    ddy <<= 1;

    if (y1 > y2) {
        SWAP(x1, x2);
        SWAP(y1, y2);
    }

    for (y = y1, x = x1; y <= y2; y++) {
        pixel(buffer, x, y, c);
        if (e < 0) {
            x--;
            e += ddy - ddx;
        }
        else {
            e -= ddx;
        }
    }
}

void bsubline_4(u8* buffer, int x1, int y1, int x2, int y2, unsigned char c) {
    int x, y, ddx, ddy, e;
    ddy = abs(y2 - y1) << 1;
    ddx = abs(x1 - x2);
    e = ddx - ddy;
    ddx <<= 1;

    if (x1 > x2) {
        SWAP(x1, x2);
        SWAP(y1, y2);
    }

    for (x = x1, y = y1; x <= x2; x++) {
        pixel(buffer, x, y, c);
        if (e < 0) {
            y--;
            e += ddx - ddy;
        }
        else {
            e -= ddy;
        }
    }
}

void line(u8* buffer, int x1, int y1, int x2, int y2, unsigned char c) {
    float k;
    if ((x1 == x2) && (y1 == y2)) {
        pixel(buffer, x1, y1, c);
        return;
    }

    if (x1 == x2) {
        vline(buffer, x1, MIN(y1, y2), MAX(y1, y2), c);
        return;
    }

    if (y1 == y2) {
        hline(buffer, MIN(x1, x2), MAX(x1, x2), y1, c);
        return;
    }

    k = (float)(y2 - y1) / (float)(x2 - x1);

    if ((k >= 0) && (k <= 1)) {
        bsubline_1(buffer, x1, y1, x2, y2, c);
    }
    else if (k > 1) {
        bsubline_2(buffer, x1, y1, x2, y2, c);
    }
    else if ((k < 0) && (k >= -1)) {
        bsubline_4(buffer, x1, y1, x2, y2, c);
    }
    else {
        bsubline_3(buffer, x1, y1, x2, y2, c);
    }
}