#ifndef _VECTORWAR_H
#define _VECTORWAR_H

#include "ggponet.h"

/*
 * vectorwar.h --
 *
 * Interface to the vector war application.
 *
 */

#define INPUT_THRUST            (1 << 0)
#define INPUT_BREAK             (1 << 1)
#define INPUT_ROTATE_LEFT       (1 << 2)
#define INPUT_ROTATE_RIGHT      (1 << 3)
#define INPUT_FIRE              (1 << 4)
#define INPUT_BOMB              (1 << 5)

void VectorWar_Init(HWND hwnd, unsigned short localport, int num_players, GGPOPlayer *players, int num_spectators);
void VectorWar_InitSpectator(HWND hwnd, unsigned short localport, int num_players, char *host_ip, unsigned short host_port);
void VectorWar_DrawCurrentFrame(void);
void VectorWar_AdvanceFrame(int inputs[], int disconnect_flags);
void VectorWar_RunFrame(HWND hwnd);
void VectorWar_Idle(int time);
void VectorWar_DisconnectPlayer(int player);
void VectorWar_Exit(void);

#if defined(GGPO_STEAM)
void VectorWar_SteamCallback(int callback_type, void *callback_data, int callback_datasize);
#endif

#define ARRAY_SIZE(n)      (sizeof(n) / sizeof(n[0]))
#define FRAME_DELAY        2

#endif
