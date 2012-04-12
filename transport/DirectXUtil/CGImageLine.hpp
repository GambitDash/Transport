/* AA Line */

#define AAlevels 256
#define AAbits 8

#define JOIN(x, y) x ## y
#define MOD(x) JOIN(x, BiColor)
#define MOD_COLOR_PARAM , Uint32 color2
#define MOD_COLOR_PARAM_RGBA , Uint8 r2, Uint8 g2, Uint8 b2, Uint8 a2
#define MOD_COLOR_P , color2
#define MOD_COLOR_P_RGBA , (((Uint32) r2 << 24) | ((Uint32) g2 << 16) | ((Uint32) b2 << 8) | (Uint32) a)
#define MOD_COLOR_VAR int redadj, greenadj, blueadj, redacc, greenacc, blueacc;
#define MOD_SWAP(tmp, c1) tmp = c1; c1 = color2; color2 = tmp;
#define MOD_COLORADJ(dc, c1, c2)                                                    \
    redadj = ((c2&0xff000000) >> 24) - ((c1&0xff000000) >> 24);                     \
    redadj = redadj << 16;                                                          \
    redadj = redadj / dc;                                                           \
    greenadj = ((c2&0xff0000) >> 16) - ((c1&0xff0000) >> 16);                       \
    greenadj = greenadj << 16;                                                      \
    greenadj = greenadj / dc;                                                       \
    blueadj = ((c2&0xff00) >> 8) - ((c1&0xff00) >> 8);                              \
    blueadj = blueadj << 16;                                                        \
    blueadj = blueadj / dc;                                                         \
    redacc = (c1 & 0xff000000) >> 8;                                                \
    greenacc = (c1 & 0xff0000);                                                     \
    blueacc = (c1 & 0xff00) << 8;

#define MOD_COLORADJ_ARGB(dc, c1, c2)                                               \
    redadj = ((c2&0xff0000) >> 24) - ((c1&0xff0000) >> 24);                         \
    redadj = redadj << 16;                                                          \
    redadj = redadj / dc;                                                           \
    greenadj = ((c2&0xff00) >> 16) - ((c1&0xff00) >> 16);                           \
    greenadj = greenadj << 16;                                                      \
    greenadj = greenadj / dc;                                                       \
    blueadj = ((c2&0xff) >> 8) - ((c1&0xff) >> 8);                                  \
    blueadj = blueadj << 16;                                                        \
    blueadj = blueadj / dc;                                                         \
    redacc = (c1 & 0xff0000) >> 8;                                                  \
    greenacc = (c1 & 0xff00);                                                       \
    blueacc = (c1 & 0xff) << 8;

#define MOD_COLORINC(c)                                                             \
    redacc += redadj;                                                               \
    greenacc += greenadj;                                                           \
    blueacc += blueadj;                                                             \
    (c) = (((Uint32) (redacc & 0xff0000)) << 8) |                                   \
        ((Uint32) ((greenacc & 0xff0000))) |                                        \
        ((Uint32) (blueacc & 0xff0000) >> 8) |                                      \
        ((Uint32) (c & 0xff));

#define MOD_COLORINC_ARGB(c)                                                        \
    redacc += redadj;                                                               \
    greenacc += greenadj;                                                           \
    blueacc += blueadj;                                                             \
    (c) = ((Uint32) (redacc & 0xff0000)) |                                          \
        (((Uint32) (greenacc & 0xff0000)) >> 8) |                                   \
        (((Uint32) (blueacc & 0xff0000)) >> 16) |                                   \
        ((Uint32) (c & 0xff000000));

#define MOD_OPT(x) x
#define MOD_COLOR_MAP {                                                             \
    Uint8 *cp = (Uint8 *) & color2;                                                 \
    if (SDL_BYTEORDER == SDL_BIG_ENDIAN) {                                          \
        color2 = SDL_MapRGBA(dst->format, cp[0], cp[1], cp[2], cp[3]);              \
    } else {                                                                        \
        color2 = SDL_MapRGBA(dst->format, cp[3], cp[2], cp[1], cp[0]);              \
    }                                                                               \
}

#include "CGImageLineCommon.h"

/* ----- Line */

/* Non-alpha line drawing code adapted from routine          */
/* by Pete Shinners, pete@shinners.org                       */
/* Originally from pygame, http://pygame.seul.org            */

#define ABS(a) (((a)<0) ? -(a) : (a))
int MOD(lineColor)(SDL_Surface * dst, Sint16 x1, Sint16 y1, Sint16 x2, Sint16 y2, Uint32 color1 MOD_COLOR_PARAM);
int MOD(lineRGBA)(SDL_Surface * dst, Sint16 x1, Sint16 y1, Sint16 x2, Sint16 y2, Uint8 r, Uint8 g, Uint8 b, Uint8 a MOD_COLOR_PARAM_RGBA);
int MOD(hlineColorStore)(SDL_Surface * dst, Sint16 x1, Sint16 x2, Sint16 y, Uint32 color1 MOD_COLOR_PARAM);
int MOD(hlineRGBAStore)(SDL_Surface * dst, Sint16 x1, Sint16 x2, Sint16 y, Uint8 r, Uint8 g, Uint8 b, Uint8 a MOD_COLOR_PARAM_RGBA);
int MOD(hlineColor)(SDL_Surface * dst, Sint16 x1, Sint16 x2, Sint16 y, Uint32 color1 MOD_COLOR_PARAM);
int MOD(hlineRGBA)(SDL_Surface * dst, Sint16 x1, Sint16 x2, Sint16 y, Uint8 r, Uint8 g, Uint8 b, Uint8 a MOD_COLOR_PARAM_RGBA);
int MOD(vlineColor)(SDL_Surface * dst, Sint16 x, Sint16 y1, Sint16 y2, Uint32 color1 MOD_COLOR_PARAM);
int MOD(vlineRGBA)(SDL_Surface * dst, Sint16 x, Sint16 y1, Sint16 y2, Uint8 r, Uint8 g, Uint8 b, Uint8 a MOD_COLOR_PARAM_RGBA);
int MOD(aalineColorInt)(SDL_Surface * dst, Sint16 x1, Sint16 y1, Sint16 x2, Sint16 y2, Uint32 color1
                MOD_COLOR_PARAM, int draw_endpoint);
int MOD(aalineColor)(SDL_Surface * dst, Sint16 x1, Sint16 y1, Sint16 x2, Sint16 y2, Uint32 color1 MOD_COLOR_PARAM);
int MOD(aalineRGBA)(SDL_Surface * dst, Sint16 x1, Sint16 y1, Sint16 x2, Sint16 y2,
        Uint8 r, Uint8 g, Uint8 b, Uint8 a MOD_COLOR_PARAM_RGBA);
extern "C" int HLineAlpha(SDL_Surface * dst, Sint16 x1, Sint16 x2, Sint16 y, Uint32 color);
extern "C" int VLineAlpha(SDL_Surface * dst, Sint16 x, Sint16 y1, Sint16 y2, Uint32 color);

int MOD(lineColor)(SDL_Surface * dst, Sint16 x1, Sint16 y1, Sint16 x2, Sint16 y2, Uint32 color1 MOD_COLOR_PARAM)
{
    Uint32 color = color1;
    int pixx, pixy;
    int x, y;
    int dx, dy;
    int ax, ay;
    int sx, sy;
    int swaptmp;
    Uint8 *pixel;
    Uint8 *colorptr;
    BOOL bFlip = FALSE;
    MOD_COLOR_VAR;

    /*
     * Clip line and test if we have to draw 
     */
    if (!(clipLine(dst, &x1, &y1, &x2, &y2, &bFlip))) {
        return (0);
    }

    if (bFlip) {
        MOD_SWAP(color, color1);
    }
    /*
     * Test for special cases of straight lines or single point 
     */
    if (x1 == x2) {
        if (y1 < y2) {
            return (MOD(vlineColor)(dst, x1, y1, y2, color1 MOD_COLOR_P));
        } else if (y1 > y2) {
            MOD_SWAP(color, color1);
            return (MOD(vlineColor)(dst, x1, y2, y1, color1 MOD_COLOR_P));
        } else {
            return (pixelColor(dst, x1, y1, color));
        }
    }
    if (y1 == y2) {
        if (x1 < x2) {
            return (MOD(hlineColor)(dst, x1, x2, y1, color1 MOD_COLOR_P));
        } else if (x1 > x2) {
            MOD_SWAP(color, color1);
            return (MOD(hlineColor)(dst, x2, x1, y1, color1 MOD_COLOR_P));
        }
    }

    /*
     * Variable setup 
     */
    dx = x2 - x1;
    dy = y2 - y1;
    sx = (dx >= 0) ? 1 : -1;
    sy = (dy >= 0) ? 1 : -1;

    /* Lock surface */
    if (SDL_MUSTLOCK(dst)) {
        if (SDL_LockSurface(dst) < 0) {
            return (-1);
        }
    }

    /*
     * Check for alpha blending 
     */
    if ((color & 255) == 255) {

        /*
         * No alpha blending - use fast pixel routines 
         */

        /*
         * More variable setup 
         */
        dx = sx * dx + 1;
        dy = sy * dy + 1;
        pixx = dst->format->BytesPerPixel;
        pixy = dst->pitch;
        pixel = ((Uint8 *) dst->pixels) + pixx * (int) x1 + pixy * (int) y1;
        pixx *= sx;
        pixy *= sy;
        if (dx < dy) {
            swaptmp = dx;
            dx = dy;
            dy = swaptmp;
            swaptmp = pixx;
            pixx = pixy;
            pixy = swaptmp;
        }

        /*
         * Setup color 
         */
        MOD_COLORADJ(dx, color1, color2);
        colorptr = (Uint8 *) & color;
        if (SDL_BYTEORDER == SDL_BIG_ENDIAN) {
            color = SDL_MapRGBA(dst->format, colorptr[0], colorptr[1], colorptr[2], colorptr[3]);
        } else {
            color = SDL_MapRGBA(dst->format, colorptr[3], colorptr[2], colorptr[1], colorptr[0]);
        }

        MOD_COLOR_MAP;

        /*
         * Draw 
         */
        x = 0;
        y = 0;
        switch (dst->format->BytesPerPixel) {
        case 1:
            for (; x < dx; x++, pixel += pixx) {
                *pixel = color;
                y += dy;
                if (y >= dx) {
                    y -= dx;
                    pixel += pixy;
                }
            }
            break;
        case 2:
            for (; x < dx; x++, pixel += pixx) {
                *(Uint16 *) pixel = color;
                y += dy;
                if (y >= dx) {
                    y -= dx;
                    pixel += pixy;
                }
            }
            break;
        case 3:
            for (; x < dx; x++, pixel += pixx) {
                if (SDL_BYTEORDER == SDL_BIG_ENDIAN) {
                    pixel[0] = (color >> 16) & 0xff;
                    pixel[1] = (color >> 8) & 0xff;
                    pixel[2] = color & 0xff;
                } else {
                    pixel[0] = color & 0xff;
                    pixel[1] = (color >> 8) & 0xff;
                    pixel[2] = (color >> 16) & 0xff;
                }
                y += dy;
                if (y >= dx) {
                    y -= dx;
                    pixel += pixy;
                }
            }
            break;
        default:                /* case 4 */
            for (; x < dx; x++, pixel += pixx) {
                *(Uint32 *) pixel = color;
                y += dy;
                if (y >= dx) {
                    y -= dx;
                    pixel += pixy;
                }
                MOD_COLORINC_ARGB(color);
            }
            break;
        }

    } else {

        /*
         * Alpha blending required - use single-pixel blits 
         */

        ax = ABS(dx) << 1;
        ay = ABS(dy) << 1;
        x = x1;
        y = y1;
        if (ax > ay) {
            int d = ay - (ax >> 1);

            while (x != x2) {
                pixelColorNolock (dst, x, y, color);
                if (d > 0 || (d == 0 && sx == 1)) {
                    y += sy;
                    d -= ax;
                }
                x += sx;
                d += ay;
                MOD_COLORINC(color);
            }
        } else {
            int d = ax - (ay >> 1);

            while (y != y2) {
                pixelColorNolock (dst, x, y, color);
                if (d > 0 || ((d == 0) && (sy == 1))) {
                    x += sx;
                    d -= ay;
                }
                y += sy;
                d += ax;
                MOD_COLORINC(color);
            }
        }
        pixelColorNolock (dst, x, y, color2);

    }

    /* Unlock surface */
    if (SDL_MUSTLOCK(dst)) {
        SDL_UnlockSurface(dst);
    }

    return (0);
}

int MOD(lineRGBA)(SDL_Surface * dst, Sint16 x1, Sint16 y1, Sint16 x2, Sint16 y2, Uint8 r, Uint8 g, Uint8 b, Uint8 a MOD_COLOR_PARAM_RGBA)
{
    /*
     * Draw 
     */
    return (MOD(lineColor)(dst, x1, y1, x2, y2, ((Uint32) r << 24) | ((Uint32) g << 16) | ((Uint32) b << 8) | (Uint32) a MOD_COLOR_P_RGBA));
}

/* ----- Horizontal line */

/* Just store color including alpha, no blending */

int MOD(hlineColorStore)(SDL_Surface * dst, Sint16 x1, Sint16 x2, Sint16 y, Uint32 color1 MOD_COLOR_PARAM)
{
    Uint32 color;
    Sint16 left, right, top, bottom;
    Uint8 *pixel, *pixellast;
    int dx;
    int pixx, pixy;
    Sint16 w;
    Sint16 xtmp;
    int result = -1;
    MOD_COLOR_VAR;

    /*
     * Check visibility of clipping rectangle
     */
    if ((dst->clip_rect.w==0) || (dst->clip_rect.h==0)) {
        return(0);
    }
    
    /*
     * Swap x1, x2 if required to ensure x1<=x2
     */
    if (x1 > x2) {
        xtmp = x1;
        x1 = x2;
        x2 = xtmp;
        MOD_SWAP(color, color1);
    }
    color = color1;

    /*
     * Get clipping boundary and
     * check visibility of hline 
     */
    left = dst->clip_rect.x;
    if (x2<left) {
        return(0);
    }
    right = dst->clip_rect.x + dst->clip_rect.w - 1;
    if (x1>right) {
        return(0);
    }
    top = dst->clip_rect.y;
    bottom = dst->clip_rect.y + dst->clip_rect.h - 1;
    if ((y<top) || (y>bottom)) {
        return (0);
    }

    /*
     * Clip x 
     */
    if (x1 < left) {
        x1 = left;
    }
    if (x2 > right) {
        x2 = right;
    }

    /*
     * Calculate width 
     */
    w = x2 - x1;

    MOD_COLORADJ_ARGB(w, color1, color2);
    /*
     * Lock surface 
     */
    SDL_LockSurface(dst);

    /*
     * More variable setup 
     */
    dx = w;
    pixx = dst->format->BytesPerPixel;
    pixy = dst->pitch;
    pixel = ((Uint8 *) dst->pixels) + pixx * (int) x1 + pixy * (int) y;

    /*
    * Draw 
    */
    switch (dst->format->BytesPerPixel) {
        case 1:
            memset(pixel, color, dx);
            break;
        case 2:
            pixellast = pixel + dx + dx;
            for (; pixel <= pixellast; pixel += pixx) {
                *(Uint16 *) pixel = color;
            }
            break;
        case 3:
            pixellast = pixel + dx + dx + dx;
            for (; pixel <= pixellast; pixel += pixx) {
                if (SDL_BYTEORDER == SDL_BIG_ENDIAN) {
                    pixel[0] = (color >> 16) & 0xff;
                    pixel[1] = (color >> 8) & 0xff;
                    pixel[2] = color & 0xff;
                } else {
                    pixel[0] = color & 0xff;
                    pixel[1] = (color >> 8) & 0xff;
                    pixel[2] = (color >> 16) & 0xff;
                }
            }
            break;
        default:                /* case 4 */
            dx = dx + dx;
            pixellast = pixel + dx + dx;
            for (; pixel <= pixellast; pixel += pixx) {
                MOD_COLORINC_ARGB(color);
                *(Uint32 *) pixel = color;
            }
            break;
        }

        /*
         * Unlock surface 
         */
        SDL_UnlockSurface(dst);

        /*
         * Set result code 
         */
        result = 0;

    return (result);
}

int MOD(hlineRGBAStore)(SDL_Surface * dst, Sint16 x1, Sint16 x2, Sint16 y, Uint8 r, Uint8 g, Uint8 b, Uint8 a MOD_COLOR_PARAM_RGBA)
{
    /*
     * Draw 
     */
    return (MOD(hlineColorStore)(dst, x1, x2, y, ((Uint32) r << 24) | ((Uint32) g << 16) | ((Uint32) b << 8) | (Uint32) a MOD_COLOR_P_RGBA));
}

int MOD(hlineColor)(SDL_Surface * dst, Sint16 x1, Sint16 x2, Sint16 y, Uint32 color1 MOD_COLOR_PARAM)
{
    Uint32 color;
    Sint16 left, right, top, bottom;
    Uint8 *pixel, *pixellast;
    int dx;
    int pixx, pixy;
    Sint16 w;
    Sint16 xtmp;
    int result = -1;
    Uint8 *colorptr;
    MOD_COLOR_VAR;

    /*
     * Check visibility of clipping rectangle
     */
    if ((dst->clip_rect.w==0) || (dst->clip_rect.h==0)) {
        return(0);
    }
    
    /*
     * Swap x1, x2 if required to ensure x1<=x2
     */
    if (x1 > x2) {
        xtmp = x1;
        x1 = x2;
        x2 = xtmp;
        MOD_SWAP(color, color1);
    }
    color = color1;

    /*
     * Get clipping boundary and
     * check visibility of hline 
     */
    left = dst->clip_rect.x;
    if (x2<left) {
        return(0);
    }
    right = dst->clip_rect.x + dst->clip_rect.w - 1;
    if (x1>right) {
        return(0);
    }
    top = dst->clip_rect.y;
    bottom = dst->clip_rect.y + dst->clip_rect.h - 1;
    if ((y<top) || (y>bottom)) {
        return (0);
    }

    /*
     * Clip x 
     */
    if (x1 < left) {
        x1 = left;
    }
    if (x2 > right) {
        x2 = right;
    }

    /*
     * Calculate width 
     */
    w = x2 - x1;

    /*
     * Alpha check 
     */
    if ((color & 255) == 255) {

        /*
         * No alpha-blending required 
         */

        /*
         * Setup color 
         */
        MOD_COLORADJ(w, color1, color2);
        colorptr = (Uint8 *) & color;
        if (SDL_BYTEORDER == SDL_BIG_ENDIAN) {
            color = SDL_MapRGBA(dst->format, colorptr[0], colorptr[1], colorptr[2], colorptr[3]);
        } else {
            color = SDL_MapRGBA(dst->format, colorptr[3], colorptr[2], colorptr[1], colorptr[0]);
        }

        MOD_COLOR_MAP;

        /*
         * Lock surface 
         */
        SDL_LockSurface(dst);

        /*
         * More variable setup 
         */
        dx = w;
        pixx = dst->format->BytesPerPixel;
        pixy = dst->pitch;
        pixel = ((Uint8 *) dst->pixels) + pixx * (int) x1 + pixy * (int) y;

        /*
         * Draw 
         */
        switch (dst->format->BytesPerPixel) {
        case 1:
            memset(pixel, color, dx);
            break;
        case 2:
            pixellast = pixel + dx + dx;
            for (; pixel <= pixellast; pixel += pixx) {
                *(Uint16 *) pixel = color;
            }
            break;
        case 3:
            pixellast = pixel + dx + dx + dx;
            for (; pixel <= pixellast; pixel += pixx) {
                if (SDL_BYTEORDER == SDL_BIG_ENDIAN) {
                    pixel[0] = (color >> 16) & 0xff;
                    pixel[1] = (color >> 8) & 0xff;
                    pixel[2] = color & 0xff;
                } else {
                    pixel[0] = color & 0xff;
                    pixel[1] = (color >> 8) & 0xff;
                    pixel[2] = (color >> 16) & 0xff;
                }
            }
            break;
        default:                /* case 4 */
            dx = dx + dx;
            pixellast = pixel + dx + dx;
            for (; pixel <= pixellast; pixel += pixx) {
                *(Uint32 *) pixel = color;
                MOD_COLORINC_ARGB(color);
            }
            break;
        }

        /*
         * Unlock surface 
         */
        SDL_UnlockSurface(dst);

        /*
         * Set result code 
         */
        result = 0;

    } else {

        /*
         * Alpha blending blit 
         */

        result = HLineAlpha(dst, x1, x1 + w, y, color);

    }

    return (result);
}

int MOD(hlineRGBA)(SDL_Surface * dst, Sint16 x1, Sint16 x2, Sint16 y, Uint8 r, Uint8 g, Uint8 b, Uint8 a MOD_COLOR_PARAM_RGBA)
{
    /*
     * Draw 
     */
    return (MOD(hlineColor)(dst, x1, x2, y, ((Uint32) r << 24) | ((Uint32) g << 16) | ((Uint32) b << 8) | (Uint32) a MOD_COLOR_P_RGBA));
}

/* ----- Vertical line */

int MOD(vlineColor)(SDL_Surface * dst, Sint16 x, Sint16 y1, Sint16 y2, Uint32 color1 MOD_COLOR_PARAM)
{
    Uint32 color;
    Sint16 left, right, top, bottom;
    Uint8 *pixel, *pixellast;
    int dy;
    int pixx, pixy;
    Sint16 h;
    Sint16 ytmp;
    int result = -1;
    Uint8 *colorptr;
    MOD_COLOR_VAR;

    /*
     * Check visibility of clipping rectangle
     */
    if ((dst->clip_rect.w==0) || (dst->clip_rect.h==0)) {
        return(0);
    }
    
    /*
     * Swap y1, y2 if required to ensure y1<=y2
     */
    if (y1 > y2) {
        ytmp = y1;
        y1 = y2;
        y2 = ytmp;
        MOD_SWAP(color, color1);
    }
    color = color1;

    /*
     * Get clipping boundary and
     * check visibility of vline 
     */
    left = dst->clip_rect.x;
    right = dst->clip_rect.x + dst->clip_rect.w - 1;
    if ((x<left) || (x>right)) {
        return (0);
    }    
    top = dst->clip_rect.y;
    if (y2<top) {
        return(0);
    }
    bottom = dst->clip_rect.y + dst->clip_rect.h - 1;
    if (y1>bottom) {
        return(0);
    }

    /*
     * Clip x 
     */
    if (y1 < top) {
        y1 = top;
    }
    if (y2 > bottom) {
        y2 = bottom;
    }

    /*
     * Calculate height
     */
    h = y2 - y1;

    /*
     * Alpha check 
     */
    if ((color & 255) == 255) {

        /*
         * No alpha-blending required 
         */

        /*
         * Setup color 
         */
        MOD_COLORADJ(h, color1, color2);
        colorptr = (Uint8 *) & color;
        if (SDL_BYTEORDER == SDL_BIG_ENDIAN) {
            color = SDL_MapRGBA(dst->format, colorptr[0], colorptr[1], colorptr[2], colorptr[3]);
        } else {
            color = SDL_MapRGBA(dst->format, colorptr[3], colorptr[2], colorptr[1], colorptr[0]);
        }

        MOD_COLOR_MAP;

        /*
         * Lock surface 
         */
        SDL_LockSurface(dst);

        /*
         * More variable setup 
         */
        dy = h;
        pixx = dst->format->BytesPerPixel;
        pixy = dst->pitch;
        pixel = ((Uint8 *) dst->pixels) + pixx * (int) x + pixy * (int) y1;
        pixellast = pixel + pixy * dy;

        /*
         * Draw 
         */
        switch (dst->format->BytesPerPixel) {
        case 1:
            for (; pixel <= pixellast; pixel += pixy) {
                *(Uint8 *) pixel = color;
            }
            break;
        case 2:
            for (; pixel <= pixellast; pixel += pixy) {
                *(Uint16 *) pixel = color;
            }
            break;
        case 3:
            for (; pixel <= pixellast; pixel += pixy) {
                if (SDL_BYTEORDER == SDL_BIG_ENDIAN) {
                    pixel[0] = (color >> 16) & 0xff;
                    pixel[1] = (color >> 8) & 0xff;
                    pixel[2] = color & 0xff;
                } else {
                    pixel[0] = color & 0xff;
                    pixel[1] = (color >> 8) & 0xff;
                    pixel[2] = (color >> 16) & 0xff;
                }
            }
            break;
        default:                /* case 4 */
            for (; pixel <= pixellast; pixel += pixy) {
                *(Uint32 *) pixel = color;
                MOD_COLORINC_ARGB(color);
            }
            break;
        }

        /*
         * Unlock surface 
         */
        SDL_UnlockSurface(dst);

        /*
         * Set result code 
         */
        result = 0;

    } else {

        /*
         * Alpha blending blit 
         */

        result = VLineAlpha(dst, x, y1, y1 + h, color);

    }

    return (result);
}

int MOD(vlineRGBA)(SDL_Surface * dst, Sint16 x, Sint16 y1, Sint16 y2, Uint8 r, Uint8 g, Uint8 b, Uint8 a MOD_COLOR_PARAM_RGBA)
{
    /*
     * Draw 
     */
    return (MOD(vlineColor)(dst, x, y1, y2, ((Uint32) r << 24) | ((Uint32) g << 16) | ((Uint32) b << 8) | (Uint32) a MOD_COLOR_P_RGBA));
}
/* 

This implementation of the Wu antialiasing code is based on Mike Abrash's
DDJ article which was reprinted as Chapter 42 of his Graphics Programming
Black Book, but has been optimized to work with SDL and utilizes 32-bit
fixed-point arithmetic. (A. Schiffler).

*/

int MOD(aalineColorInt)(SDL_Surface * dst, Sint16 x1, Sint16 y1, Sint16 x2, Sint16 y2, Uint32 color1
                MOD_COLOR_PARAM, int draw_endpoint)
{
    Uint32 color;
    Sint32 xx0, yy0, xx1, yy1;
    int result;
    Uint32 intshift, erracc, erradj;
    Uint32 erracctmp, wgt, wgtcompmask;
    int dx, dy, tmp, xdir, y0p1, x0pxdir;
    MOD_COLOR_VAR;
    BOOL bFlip = FALSE;
    BOOL bReor = FALSE;

    /*
     * Check visibility of clipping rectangle
     */
    if ((dst->clip_rect.w==0) || (dst->clip_rect.h==0)) {
        return(0);
    }

    /*
     * Clip line and test if we have to draw 
     */
    if (!(clipLine(dst, &x1, &y1, &x2, &y2, &bFlip))) {
        return (0);
    }

    /*
     * Keep on working with 32bit numbers 
     */
    xx0 = x1;
    yy0 = y1;
    xx1 = x2;
    yy1 = y2;

    /*
     * Reorder points if required 
     */
    if (yy0 > yy1) {
        tmp = yy0;
        yy0 = yy1;
        yy1 = tmp;
        tmp = xx0;
        xx0 = xx1;
        xx1 = tmp;
        bReor = TRUE;
    }


    /*
     * Calculate distance 
     */
    dx = xx1 - xx0;
    dy = yy1 - yy0;

    /*
     * Adjust for negative dx and set xdir 
     */
    if (dx >= 0) {
        xdir = 1;
    } else {
        xdir = -1;
        dx = (-dx);
    }

    color = color1;

    /*
     * Check for special cases 
     */
    if (dx == 0) {
        /*
         * Vertical line 
         */
        if (bFlip) {
            MOD_SWAP(color, color1);
        }
        return (MOD(vlineColor)(dst, x1, y1, y2, color1 MOD_COLOR_P));
    } else if (dy == 0) {
        /*
         * Horizontal line 
         */
        if (bFlip) {
            MOD_SWAP(color, color1);
        }
        return (MOD(hlineColor)(dst, x1, x2, y1, color1 MOD_COLOR_P));
    } else if (dx == dy) {
        /*
         * Diagonal line 
         */
        return (MOD(lineColor)(dst, x1, y1, x2, y2, color1 MOD_COLOR_P));
    }

    /* Reorder the colors if necessary. */
    if ((!bFlip && bReor) || (xdir < 0 && bFlip && !bReor)) {
        MOD_SWAP(color, color1);
    }
    color = color1;

    //printf("(%x, %x, %x) -> (%x, %x, %x)\n", (color1 & 0xff000000) >> 24, (color1 & 0xff0000) >> 16, (color1 & 0xff00) >> 8,
    //    (color2 & 0xff000000) >> 24, (color2 & 0xff0000) >> 16, (color2 & 0xff00) >> 8);


    /*
     * Line is not horizontal, vertical or diagonal 
     */
    result = 0;

    /*
     * Zero accumulator 
     */
    erracc = 0;

    /*
     * # of bits by which to shift erracc to get intensity level 
     */
    intshift = 32 - AAbits;
    /*
     * Mask used to flip all bits in an intensity weighting 
     */
    wgtcompmask = AAlevels - 1;

    /* Lock surface */
    if (SDL_MUSTLOCK(dst)) {
        if (SDL_LockSurface(dst) < 0) {
            return (-1);
        }
    }

    /*
     * Draw the initial pixel in the foreground color 
     */
    result |= pixelColorNolock(dst, x1, y1, bReor ? color2 : color1);

    /*
     * x-major or y-major? 
     */
    if (dy > dx) {

        /*
         * y-major.  Calculate 16-bit fixed point fractional part of a pixel that
         * X advances every time Y advances 1 pixel, truncating the result so that
         * we won't overrun the endpoint along the X axis 
         */
        /*
         * Not-so-portable version: erradj = ((Uint64)dx << 32) / (Uint64)dy; 
         */
        erradj = ((dx << 16) / dy) << 16;

        MOD_COLORADJ(dy, color1, color2);
          //printf("%d steps (%d, %d, %d) + (%f, %f, %f)\n", dy, redacc, greenacc, blueacc, redadj / 65535.0, greenadj / 65535.0, blueadj / 65535.0);
        /*
         * draw all pixels other than the first and last 
         */
        x0pxdir = xx0 + xdir;
        while (--dy) {
            erracctmp = erracc;
            erracc += erradj;
            if (erracc <= erracctmp) {
                /*
                 * rollover in error accumulator, x coord advances 
                 */
                xx0 = x0pxdir;
                x0pxdir += xdir;
            }
            yy0++;                /* y-major so always advance Y */

            MOD_COLORINC(color);

            /*
             * the AAbits most significant bits of erracc give us the intensity
             * weighting for this pixel, and the complement of the weighting for
             * the paired pixel. 
             */
            wgt = (erracc >> intshift) & 255;
            result |= pixelColorWeightNolock (dst, xx0, yy0, color, 255 - wgt);
            result |= pixelColorWeightNolock (dst, x0pxdir, yy0, color, wgt);
        }

    } else {

        /*
         * x-major line.  Calculate 16-bit fixed-point fractional part of a pixel
         * that Y advances each time X advances 1 pixel, truncating the result so
         * that we won't overrun the endpoint along the X axis. 
         */
        /*
         * Not-so-portable version: erradj = ((Uint64)dy << 32) / (Uint64)dx; 
         */
        erradj = ((dy << 16) / dx) << 16;

        MOD_COLORADJ(dx, color1, color2);

       //printf("%d steps (%x, %x, %x) + (%f, %f, %f) or (%x, %x, %x)\n", dx, redacc, greenacc, blueacc, redadj / 65535.0, greenadj / 65535.0, blueadj / 65535.0, redadj, greenadj, blueadj);
      /*
         * draw all pixels other than the first and last 
         */
        y0p1 = yy0 + 1;
        while (--dx) {

            erracctmp = erracc;
            erracc += erradj;
            if (erracc <= erracctmp) {
                /*
                 * Accumulator turned over, advance y 
                 */
                yy0 = y0p1;
                y0p1++;
            }
            xx0 += xdir;        /* x-major so always advance X */

            MOD_COLORINC(color);

            /*
             * the AAbits most significant bits of erracc give us the intensity
             * weighting for this pixel, and the complement of the weighting for
             * the paired pixel. 
             */
            wgt = (erracc >> intshift) & 255;
            result |= pixelColorWeightNolock (dst, xx0, yy0, color, 255 - wgt);
            result |= pixelColorWeightNolock (dst, xx0, y0p1, color, wgt);
        }
    }

    /*
     * Do we have to draw the endpoint 
     */
    if (draw_endpoint) {
        /*
         * Draw final pixel, always exactly intersected by the line and doesn't
         * need to be weighted. 
         */
        result |= pixelColorNolock (dst, x2, y2, bReor ? color1 : color2);
    }

    /* Unlock surface */
    if (SDL_MUSTLOCK(dst)) {
        SDL_UnlockSurface(dst);
    }

    return (result);
}

int MOD(aalineColor)(SDL_Surface * dst, Sint16 x1, Sint16 y1, Sint16 x2, Sint16 y2, Uint32 color1 MOD_COLOR_PARAM)
{
    return MOD(aalineColorInt)(dst, x1, y1, x2, y2, color1 MOD_COLOR_P, 1);
}

int MOD(aalineRGBA)(SDL_Surface * dst, Sint16 x1, Sint16 y1, Sint16 x2, Sint16 y2,
        Uint8 r, Uint8 g, Uint8 b, Uint8 a MOD_COLOR_PARAM_RGBA)
{
    return (MOD(aalineColorInt)
            (dst, x1, y1, x2, y2,
             (((Uint32) r << 24) | ((Uint32) g << 16) | ((Uint32) b << 8) | (Uint32) a) MOD_COLOR_P_RGBA, 1));
}
