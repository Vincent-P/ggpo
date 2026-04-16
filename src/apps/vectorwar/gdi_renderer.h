#ifndef _GDI_RENDERER_H_
#define _GDI_RENDERER_H_

#include "renderer.h"

/*
 * gdi_renderer.h --
 *
 * A simple C renderer that uses GDI to render the game state.
 */

typedef struct GDIRenderer {
   /* Must be first member so we can cast Renderer* <-> GDIRenderer* */
   Renderer     base;

   HFONT        _font;
   HWND         _hwnd;
   RECT         _rc;
   char         _status[1024];
   COLORREF     _shipColors[4];
   HPEN         _shipPens[4];
   HBRUSH       _bulletBrush;
   HBRUSH       _redBrush;
} GDIRenderer;

GDIRenderer *GDIRenderer_Create(HWND hwnd);

#endif
