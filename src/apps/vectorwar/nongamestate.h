#ifndef _NON_GAMESTATE_H_
#define _NON_GAMESTATE_H_

#include "ggponet.h"

#define MAX_PLAYERS     64

/*
 * nongamestate.h --
 *
 * These are other pieces of information not related to the state
 * of the game which are useful to carry around.  They are not
 * included in the GameState struct because they specifically
 * should not be rolled back.
 */

typedef enum PlayerConnectState {
   Connecting = 0,
   Synchronizing,
   Running,
   Disconnected,
   Disconnecting,
} PlayerConnectState;

typedef struct PlayerConnectionInfo {
   GGPOPlayerType       type;
   GGPOPlayerHandle     handle;
   PlayerConnectState   state;
   int                  connect_progress;
   int                  disconnect_timeout;
   int                  disconnect_start;
} PlayerConnectionInfo;

typedef struct ChecksumInfo {
   int framenumber;
   int checksum;
} ChecksumInfo;

typedef struct NonGameState {
   GGPOPlayerHandle     local_player_handle;
   PlayerConnectionInfo players[MAX_PLAYERS];
   int                  num_players;

   ChecksumInfo         now;
   ChecksumInfo         periodic;
} NonGameState;

static __inline void NonGameState_SetConnectStateByHandle(NonGameState *ngs, GGPOPlayerHandle handle, PlayerConnectState state)
{
   int i;
   for (i = 0; i < ngs->num_players; i++) {
      if (ngs->players[i].handle == handle) {
         ngs->players[i].connect_progress = 0;
         ngs->players[i].state = state;
         break;
      }
   }
}

static __inline void NonGameState_SetDisconnectTimeout(NonGameState *ngs, GGPOPlayerHandle handle, int when, int timeout)
{
   int i;
   for (i = 0; i < ngs->num_players; i++) {
      if (ngs->players[i].handle == handle) {
         ngs->players[i].disconnect_start = when;
         ngs->players[i].disconnect_timeout = timeout;
         ngs->players[i].state = Disconnecting;
         break;
      }
   }
}

static __inline void NonGameState_SetConnectStateAll(NonGameState *ngs, PlayerConnectState state)
{
   int i;
   for (i = 0; i < ngs->num_players; i++) {
      ngs->players[i].state = state;
   }
}

static __inline void NonGameState_UpdateConnectProgress(NonGameState *ngs, GGPOPlayerHandle handle, int progress)
{
   int i;
   for (i = 0; i < ngs->num_players; i++) {
      if (ngs->players[i].handle == handle) {
         ngs->players[i].connect_progress = progress;
         break;
      }
   }
}

#endif
