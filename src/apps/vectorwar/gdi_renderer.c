#include <windows.h>
#include <stdio.h>
#include <math.h>
#include "vectorwar.h"
#include "gdi_renderer.h"

#define  PROGRESS_BAR_WIDTH        100
#define  PROGRESS_BAR_TOP_OFFSET    22
#define  PROGRESS_BAR_HEIGHT         8
#define  PROGRESS_TEXT_OFFSET       (PROGRESS_BAR_TOP_OFFSET + PROGRESS_BAR_HEIGHT + 4)

static void GDIRenderer_CreateGDIFont(GDIRenderer *r, HDC hdc);
static void GDIRenderer_RenderChecksum(GDIRenderer *r, HDC hdc, int y, ChecksumInfo *info);
static void GDIRenderer_DrawShip(GDIRenderer *r, HDC hdc, int which, GameState *gs);
static void GDIRenderer_DrawConnectState(GDIRenderer *r, HDC hdc, Ship *ship, PlayerConnectionInfo *info);

static void
GDIRenderer_Draw(Renderer *self, GameState *gs, NonGameState *ngs)
{
   GDIRenderer *r = (GDIRenderer *)self;
   HDC hdc = GetDC(r->_hwnd);
   int i;

   FillRect(hdc, &r->_rc, (HBRUSH)GetStockObject(BLACK_BRUSH));
   FrameRect(hdc, &gs->_bounds, (HBRUSH)GetStockObject(WHITE_BRUSH));

   SetBkMode(hdc, TRANSPARENT);
   SelectObject(hdc, r->_font);

   for (i = 0; i < gs->_num_ships; i++) {
      SetTextColor(hdc, r->_shipColors[i]);
      SelectObject(hdc, r->_shipPens[i]);
      GDIRenderer_DrawShip(r, hdc, i, gs);
      GDIRenderer_DrawConnectState(r, hdc, &gs->_ships[i], &ngs->players[i]);
   }

   SetTextAlign(hdc, TA_BOTTOM | TA_CENTER);
   TextOutA(hdc, (r->_rc.left + r->_rc.right) / 2, r->_rc.bottom - 32, r->_status, (int)strlen(r->_status));

   SetTextColor(hdc, RGB(192, 192, 192));
   GDIRenderer_RenderChecksum(r, hdc, 40, &ngs->periodic);
   SetTextColor(hdc, RGB(128, 128, 128));
   GDIRenderer_RenderChecksum(r, hdc, 56, &ngs->now);
#if defined(GGPO_STEAM)
   
#define S_API __declspec( dllimport )
#define S_CALLTYPE __cdecl
typedef struct ISteamUser ISteamUser;
typedef uint64_t uint64_steamid;
   // A versioned accessor is exported by the library
S_API ISteamUser* SteamAPI_SteamUser_v023();
S_API uint64_steamid SteamAPI_ISteamUser_GetSteamID(ISteamUser* self);

   {
       SetTextColor(hdc, RGB(192, 192, 192));
       int y = 72;
	    uint64_steamid steam_id = SteamAPI_ISteamUser_GetSteamID(SteamAPI_SteamUser_v023());
       char user_id[128];
       sprintf_s(user_id, ARRAYSIZE(user_id), "User ID: %llu", steam_id);
       TextOutA(hdc, (r->_rc.left + r->_rc.right) / 2, r->_rc.top + y, user_id, (int)strlen(user_id));
   }

#endif

   ReleaseDC(r->_hwnd, hdc);
}

static void
GDIRenderer_RenderChecksum(GDIRenderer *r, HDC hdc, int y, ChecksumInfo *info)
{
   char checksum[128];
   sprintf_s(checksum, ARRAYSIZE(checksum), "Frame: %04d  Checksum: %08x", info->framenumber, info->checksum);
   TextOutA(hdc, (r->_rc.left + r->_rc.right) / 2, r->_rc.top + y, checksum, (int)strlen(checksum));
}


static void
GDIRenderer_SetStatusText(Renderer *self, const char *text)
{
   GDIRenderer *r = (GDIRenderer *)self;
   strcpy_s(r->_status, sizeof(r->_status), text);
}

static void
GDIRenderer_DrawShip(GDIRenderer *r, HDC hdc, int which, GameState *gs)
{
   Ship *ship = gs->_ships + which;
   RECT bullet = { 0 };
   POINT shape[] = {
      { SHIP_RADIUS,           0 },
      { -SHIP_RADIUS,          SHIP_WIDTH },
      { SHIP_TUCK-SHIP_RADIUS, 0 },
      { -SHIP_RADIUS,          -SHIP_WIDTH },
      { SHIP_RADIUS,           0 },
   };
   int alignments[] = {
      TA_TOP | TA_LEFT,
      TA_TOP | TA_RIGHT,
      TA_BOTTOM | TA_LEFT,
      TA_BOTTOM | TA_RIGHT,
   };
   POINT text_offsets[] = {
      { gs->_bounds.left  + 2, gs->_bounds.top + 2 },
      { gs->_bounds.right - 2, gs->_bounds.top + 2 },
      { gs->_bounds.left  + 2, gs->_bounds.bottom - 2 },
      { gs->_bounds.right - 2, gs->_bounds.bottom - 2 },
   };
   char buf[32];
   int i;

   for (i = 0; i < ARRAY_SIZE(shape); i++) {
      double newx, newy;
      double cost, sint, theta;

      theta = (double)ship->heading * PI / 180;
      cost = cos(theta);
      sint = sin(theta);

      newx = shape[i].x * cost - shape[i].y * sint;
      newy = shape[i].x * sint + shape[i].y * cost;

      shape[i].x = (LONG)(newx + ship->position.x);
      shape[i].y = (LONG)(newy + ship->position.y);
   }
   Polyline(hdc, shape, ARRAY_SIZE(shape));

   for (i = 0; i < MAX_BULLETS; i++) {
      if (ship->bullets[i].active) {
         bullet.left = (LONG)ship->bullets[i].position.x - 1;
         bullet.right = (LONG)ship->bullets[i].position.x + 1;
         bullet.top = (LONG)ship->bullets[i].position.y - 1;
         bullet.bottom = (LONG)ship->bullets[i].position.y + 1;
         FillRect(hdc, &bullet, r->_bulletBrush);
      }
   }
   SetTextAlign(hdc, alignments[which]);
   sprintf_s(buf, ARRAYSIZE(buf), "Hits: %d", ship->score);
   TextOutA(hdc, text_offsets[which].x, text_offsets[which].y, buf, (int)strlen(buf));
}

static void
GDIRenderer_DrawConnectState(GDIRenderer *r, HDC hdc, Ship *ship, PlayerConnectionInfo *info)
{
   char status[64];
   int progress = -1;

   *status = '\0';
   switch (info->state) {
      case Connecting:
         sprintf_s(status, ARRAYSIZE(status), (info->type == GGPO_PLAYERTYPE_LOCAL) ? "Local Player" : "Connecting...");
         break;

      case Synchronizing:
         progress = info->connect_progress;
         sprintf_s(status, ARRAYSIZE(status), (info->type == GGPO_PLAYERTYPE_LOCAL) ? "Local Player" : "Synchronizing...");
         break;

      case Disconnected:
         sprintf_s(status, ARRAYSIZE(status), "Disconnected");
         break;

      case Disconnecting:
         sprintf_s(status, ARRAYSIZE(status), "Waiting for player...");
         progress = (timeGetTime() - info->disconnect_start) * 100 / info->disconnect_timeout;
         break;

      default:
         break;
   }

   if (*status) {
      SetTextAlign(hdc, TA_TOP | TA_CENTER);
      TextOutA(hdc, (int)ship->position.x, (int)ship->position.y + PROGRESS_TEXT_OFFSET, status, (int)strlen(status));
   }
   if (progress >= 0) {
      HBRUSH bar = (HBRUSH)(info->state == Synchronizing ? GetStockObject(WHITE_BRUSH) : r->_redBrush);
      RECT rc;
      rc.left   = (LONG)(ship->position.x - (PROGRESS_BAR_WIDTH / 2));
      rc.top    = (LONG)(ship->position.y + PROGRESS_BAR_TOP_OFFSET);
      rc.right  = (LONG)(ship->position.x + (PROGRESS_BAR_WIDTH / 2));
      rc.bottom = (LONG)(ship->position.y + PROGRESS_BAR_TOP_OFFSET + PROGRESS_BAR_HEIGHT);

      FrameRect(hdc, &rc, (HBRUSH)GetStockObject(GRAY_BRUSH));
      rc.right = rc.left + min(100, progress) * PROGRESS_BAR_WIDTH / 100;
      InflateRect(&rc, -1, -1);
      FillRect(hdc, &rc, bar);
   }
}

static void
GDIRenderer_CreateGDIFont(GDIRenderer *r, HDC hdc)
{
   (void)hdc;
   r->_font = CreateFont(-12,
                      0,                         /* Width Of Font */
                      0,                         /* Angle Of Escapement */
                      0,                         /* Orientation Angle */
                      0,                         /* Font Weight */
                      FALSE,                     /* Italic */
                      FALSE,                     /* Underline */
                      FALSE,                     /* Strikeout */
                      ANSI_CHARSET,              /* Character Set Identifier */
                      OUT_TT_PRECIS,             /* Output Precision */
                      CLIP_DEFAULT_PRECIS,       /* Clipping Precision */
                      ANTIALIASED_QUALITY,       /* Output Quality */
                      FF_DONTCARE|DEFAULT_PITCH, /* Family And Pitch */
                      L"Tahoma");                /* Font Name */
}

static void
GDIRenderer_Destroy(Renderer *self)
{
   GDIRenderer *r = (GDIRenderer *)self;
   if (r) {
      DeleteObject(r->_font);
      {
         int i;
         for (i = 0; i < 4; i++) {
            DeleteObject(r->_shipPens[i]);
         }
      }
      DeleteObject(r->_bulletBrush);
      DeleteObject(r->_redBrush);
      free(r);
   }
}

GDIRenderer *
GDIRenderer_Create(HWND hwnd)
{
   HDC hdc;
   int i;
   GDIRenderer *r = (GDIRenderer *)malloc(sizeof(GDIRenderer));
   if (!r) return NULL;
   memset(r, 0, sizeof(GDIRenderer));

   /* Set up vtable */
   r->base.Draw = GDIRenderer_Draw;
   r->base.SetStatusText = GDIRenderer_SetStatusText;
   r->base.Destroy = GDIRenderer_Destroy;

   r->_hwnd = hwnd;
   hdc = GetDC(r->_hwnd);
   r->_status[0] = '\0';
   GetClientRect(hwnd, &r->_rc);
   GDIRenderer_CreateGDIFont(r, hdc);
   ReleaseDC(r->_hwnd, hdc);

   r->_shipColors[0] = RGB(255, 0, 0);
   r->_shipColors[1] = RGB(0, 255, 0);
   r->_shipColors[2] = RGB(0, 0, 255);
   r->_shipColors[3] = RGB(128, 128, 128);
   
   for (i = 0; i < 4; i++) {
      r->_shipPens[i] = CreatePen(PS_SOLID, 1, r->_shipColors[i]);
   }
   r->_redBrush = CreateSolidBrush(RGB(255, 0, 0));
   r->_bulletBrush = CreateSolidBrush(RGB(255, 192, 0));

   return r;
}
