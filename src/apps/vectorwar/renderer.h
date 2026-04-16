#ifndef _RENDERER_H_
#define _RENDERER_H_

#include "gamestate.h"
#include "nongamestate.h"

/*
 * renderer.h --
 *
 * Function pointer table used to render the game state.
 * Replaces the C++ abstract Renderer class.
 */

typedef struct Renderer {
   void (*Draw)(struct Renderer *self, GameState *gs, NonGameState *ngs);
   void (*SetStatusText)(struct Renderer *self, const char *text);
   void (*Destroy)(struct Renderer *self);
} Renderer;

#endif
