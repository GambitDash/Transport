#ifndef __CGIMAGELINECOMMON_H__
#define __CGIMAGELINECOMMON_H__

/* Yank and place from sdl_gfx code. */
extern "C" int pixelColorNolock(SDL_Surface * dst, Sint16 x, Sint16 y, Uint32 color);
extern "C" int pixelColorWeightNolock(SDL_Surface * dst, Sint16 x, Sint16 y, Uint32 color, Uint32 weight);

#define CLIP_LEFT_EDGE   0x1
#define CLIP_RIGHT_EDGE  0x2
#define CLIP_BOTTOM_EDGE 0x4
#define CLIP_TOP_EDGE    0x8
#define CLIP_INSIDE(a)   (!a)
#define CLIP_REJECT(a,b) (a&b)
#define CLIP_ACCEPT(a,b) (!(a|b))

static int clipEncode(Sint16 x, Sint16 y, Sint16 left, Sint16 top, Sint16 right, Sint16 bottom)
{
    int code = 0;

    if (x < left) {
        code |= CLIP_LEFT_EDGE;
    } else if (x > right) {
        code |= CLIP_RIGHT_EDGE;
    }
    if (y < top) {
        code |= CLIP_TOP_EDGE;
    } else if (y > bottom) {
        code |= CLIP_BOTTOM_EDGE;
    }
    return code;
}


static int clipLine(SDL_Surface * dst, Sint16 * x1, Sint16 * y1, Sint16 * x2, Sint16 * y2, BOOL * bFlip)
{
    Sint16 left, right, top, bottom;
    int code1, code2;
    int draw = 0;
    Sint16 swaptmp;
    float m;

    /*
     * Get clipping boundary 
     */
    left = dst->clip_rect.x;
    right = dst->clip_rect.x + dst->clip_rect.w - 1;
    top = dst->clip_rect.y;
    bottom = dst->clip_rect.y + dst->clip_rect.h - 1;

    while (1) {
        code1 = clipEncode(*x1, *y1, left, top, right, bottom);
        code2 = clipEncode(*x2, *y2, left, top, right, bottom);
        if (CLIP_ACCEPT(code1, code2)) {
            draw = 1;
            break;
        } else if (CLIP_REJECT(code1, code2))
            break;
        else {
            if (CLIP_INSIDE(code1)) {
                swaptmp = *x2;
                *x2 = *x1;
                *x1 = swaptmp;
                swaptmp = *y2;
                *y2 = *y1;
                *y1 = swaptmp;
                swaptmp = code2;
                code2 = code1;
                code1 = swaptmp;
		*bFlip = TRUE;
            }
            if (*x2 != *x1) {
                m = (*y2 - *y1) / (float) (*x2 - *x1);
            } else {
                m = 1.0f;
            }
            if (code1 & CLIP_LEFT_EDGE) {
                *y1 += (Sint16) ((left - *x1) * m);
                *x1 = left;
            } else if (code1 & CLIP_RIGHT_EDGE) {
                *y1 += (Sint16) ((right - *x1) * m);
                *x1 = right;
            } else if (code1 & CLIP_BOTTOM_EDGE) {
                if (*x2 != *x1) {
                    *x1 += (Sint16) ((bottom - *y1) / m);
                }
                *y1 = bottom;
            } else if (code1 & CLIP_TOP_EDGE) {
                if (*x2 != *x1) {
                    *x1 += (Sint16) ((top - *y1) / m);
                }
                *y1 = top;
            }
        }
    }

    return draw;
}

#endif
