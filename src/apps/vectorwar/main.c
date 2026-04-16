#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include "vectorwar.h"
#include "ggpo_perfmon.h"

#if defined(GGPO_STEAM)
#define S_API __declspec( dllimport )
#define S_CALLTYPE __cdecl
enum ESteamAPIInitResult
{
	k_ESteamAPIInitResult_OK = 0,
	k_ESteamAPIInitResult_FailedGeneric = 1, // Some other failure
	k_ESteamAPIInitResult_NoSteamClient = 2, // We cannot connect to Steam, steam probably isn't running
	k_ESteamAPIInitResult_VersionMismatch = 3, // Steam client appears to be out of date
};
typedef enum ESteamAPIInitResult ESteamAPIInitResult;
enum { k_cchMaxSteamErrMsg = 1024 };
typedef char SteamErrMsg[k_cchMaxSteamErrMsg];
S_API ESteamAPIInitResult S_CALLTYPE SteamAPI_InitFlat(SteamErrMsg* pOutErrMsg);

typedef struct ISteamUser ISteamUser;
typedef uint64_t uint64_steamid;
typedef int32_t HSteamUser;
typedef int32_t HSteamPipe;
// A versioned accessor is exported by the library
S_API ISteamUser* SteamAPI_SteamUser_v023();
// Inline, unversioned accessor to get the current version.  Essentially the same as SteamUser(), but using this ensures that you are using a matching library.
inline ISteamUser* SteamAPI_SteamUser() { return SteamAPI_SteamUser_v023(); }
S_API HSteamUser SteamAPI_ISteamUser_GetHSteamUser(ISteamUser* self);
S_API bool SteamAPI_ISteamUser_BLoggedOn(ISteamUser* self);
S_API uint64_steamid SteamAPI_ISteamUser_GetSteamID(ISteamUser* self);
#if defined(__linux__) || defined(__APPLE__) || defined(__FreeBSD__)
// The 32-bit version of gcc has the alignment requirement for uint64 and double set to
// 4 meaning that even with #pragma pack(8) these types will only be four-byte aligned.
// The 64-bit version of gcc has the alignment requirement for these types set to
// 8 meaning that unless we use #pragma pack(4) our structures will get bigger.
// The 64-bit structure packing has to match the 32-bit structure packing for each platform.
#define VALVE_CALLBACK_PACK_SMALL
#else
#define VALVE_CALLBACK_PACK_LARGE
#endif

#if defined( VALVE_CALLBACK_PACK_SMALL )
#pragma pack( push, 4 )
#elif defined( VALVE_CALLBACK_PACK_LARGE )
#pragma pack( push, 8 )
#else
#error steam_api_common.h should define VALVE_CALLBACK_PACK_xxx
#endif 

/// Internal structure used in manual callback dispatch
struct CallbackMsg_t
{
	HSteamUser m_hSteamUser; // Specific user to whom this callback applies.
	int m_iCallback; // Callback identifier.  (Corresponds to the k_iCallback enum in the callback structure.)
	uint8_t* m_pubParam; // Points to the callback structure
	int m_cubParam; // Size of the data pointed to by m_pubParam
};
typedef struct CallbackMsg_t CallbackMsg_t;
#pragma pack( pop )

S_API void S_CALLTYPE SteamAPI_ManualDispatch_Init();
S_API HSteamPipe S_CALLTYPE SteamAPI_GetHSteamPipe();
S_API void S_CALLTYPE SteamAPI_ManualDispatch_RunFrame(HSteamPipe hSteamPipe);
S_API bool S_CALLTYPE SteamAPI_ManualDispatch_GetNextCallback(HSteamPipe hSteamPipe, CallbackMsg_t* pCallbackMsg);
S_API void S_CALLTYPE SteamAPI_ManualDispatch_FreeLastCallback(HSteamPipe hSteamPipe);


typedef struct ISteamNetworkingUtils ISteamNetworkingUtils;
S_API ISteamNetworkingUtils* SteamAPI_SteamNetworkingUtils_SteamAPI_v004();
// Inline, unversioned accessor to get the current version.  Essentially the same as SteamNetworkingUtils_SteamAPI(), but using this ensures that you are using a matching library.
inline ISteamNetworkingUtils* SteamAPI_SteamNetworkingUtils_SteamAPI() { return SteamAPI_SteamNetworkingUtils_SteamAPI_v004(); }
S_API void SteamAPI_ISteamNetworkingUtils_InitRelayNetworkAccess(ISteamNetworkingUtils* self);



#endif

static LRESULT CALLBACK
MainWindowProc(HWND hwnd,
	UINT uMsg,
	WPARAM wParam,
	LPARAM lParam)
{
	switch (uMsg) {
	case WM_ERASEBKGND:
		return 1;
	case WM_KEYDOWN:
		if (wParam == 'P') {
			ggpoutil_perfmon_toggle();
		}
		else if (wParam == VK_ESCAPE) {
			VectorWar_Exit();
			PostQuitMessage(0);
		}
		else if (wParam >= VK_F1 && wParam <= VK_F12) {
			VectorWar_DisconnectPlayer((int)(wParam - VK_F1));
		}
		return 0;
	case WM_PAINT:
		VectorWar_DrawCurrentFrame();
		ValidateRect(hwnd, NULL);
		return 0;
	case WM_CLOSE:
		PostQuitMessage(0);
		break;
	}
	return CallWindowProc(DefWindowProc, hwnd, uMsg, wParam, lParam);
}

static HWND
CreateMainWindow(HINSTANCE hInstance)
{
	HWND hwnd;
	WNDCLASSEX wndclass;
	RECT rc;
	int width = 640, height = 480;
	WCHAR titlebuf[128];

	memset(&wndclass, 0, sizeof(wndclass));

	wsprintf(titlebuf, L"(pid:%d) ggpo sdk sample: vector war", GetCurrentProcessId());
	wndclass.cbSize = sizeof(wndclass);
	wndclass.lpfnWndProc = MainWindowProc;
	wndclass.lpszClassName = L"vwwnd";
	RegisterClassEx(&wndclass);
	hwnd = CreateWindow(L"vwwnd",
		titlebuf,
		WS_OVERLAPPEDWINDOW | WS_VISIBLE,
		CW_USEDEFAULT, CW_USEDEFAULT,
		width, height,
		NULL, NULL, hInstance, NULL);

	GetClientRect(hwnd, &rc);
	SetWindowPos(hwnd, NULL, 0, 0, width + (width - (rc.right - rc.left)), height + (height - (rc.bottom - rc.top)), SWP_NOMOVE);
	return hwnd;
}

static void
RunMainLoop(HWND hwnd)
{
	MSG msg;
	int start, next, now;

	memset(&msg, 0, sizeof(msg));

	start = next = now = timeGetTime();
	while (1) {
		while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
			if (msg.message == WM_QUIT) {
				return;
			}
		}
		now = timeGetTime();
		VectorWar_Idle(max(0, next - now - 1));
		if (now >= next) {
			VectorWar_RunFrame(hwnd);
			next = now + (1000 / 60);
		}

#if defined(GGPO_STEAM)
		HSteamPipe hSteamPipe = SteamAPI_GetHSteamPipe(); // See also SteamGameServer_GetHSteamPipe()
		SteamAPI_ManualDispatch_RunFrame(hSteamPipe);
		CallbackMsg_t callback;
		while (SteamAPI_ManualDispatch_GetNextCallback(hSteamPipe, &callback))
		{
			// Check for dispatching API call results
			// if (callback.m_iCallback == SteamAPICallCompleted_t::k_iCallback)
			// {
			// 	SteamAPICallCompleted_t* pCallCompleted = (SteamAPICallCompleted_t*)callback.
			// 		void* pTmpCallResult = malloc(pCallback->m_cubParam);
			// 	bool bFailed;
			// 	if (SteamAPI_ManualDispatch_GetAPICallResult(hSteamPipe, pCallCompleted->m_hAsyncCall, pTmpCallResult, pCallback->m_cubParam, pCallback->m_iCallback, &bFailed))
			// 	{
			// 		// Dispatch the call result to the registered handler(s) for the
			// 		// call identified by pCallCompleted->m_hAsyncCall
			// 	}
			// 	free(pTmpCallResult);
			// }
			// else
			{
				// Look at callback.m_iCallback to see what kind of callback it is,
				// and dispatch to appropriate handler(s)
				VectorWar_SteamCallback(callback.m_iCallback, callback.m_pubParam, callback.m_cubParam);
			}
			SteamAPI_ManualDispatch_FreeLastCallback(hSteamPipe);
		}
#endif
	}
}

static void
Syntax(void)
{
	MessageBox(NULL,
		L"Syntax: vectorwar.exe <local port> <num players> ('local' | <remote ip>:<remote port>)*\n",
		L"Could not start", MB_OK);
}

int APIENTRY wWinMain(HINSTANCE hInstance,
	HINSTANCE hPrevInstance,
	LPWSTR lpCmdLine,
	int nCmdShow)
{
#if defined(GGPO_STEAM)
	SteamErrMsg errMsg = { 0 };
	if (SteamAPI_InitFlat(&errMsg) != k_ESteamAPIInitResult_OK) {
		fprintf(stderr, "Failed to init Steam: %s.\n", errMsg);
		return 1;
	}

	SteamAPI_ISteamNetworkingUtils_InitRelayNetworkAccess(SteamAPI_SteamNetworkingUtils_SteamAPI());

	SteamAPI_ManualDispatch_Init();

	uint64_steamid user_id = SteamAPI_ISteamUser_GetSteamID(SteamAPI_SteamUser());
	fprintf(stdout, "Init steam success: user id %llu.\n", user_id);

#endif

	HWND hwnd;
	int offset = 1, local_player = 0;
	WSADATA wd;
	wchar_t wide_ip_buffer[128];
	unsigned int wide_ip_buffer_size = (unsigned int)ARRAYSIZE(wide_ip_buffer);
	unsigned short local_port;
	int num_players;

	POINT window_offsets[] = {
	   { 64,  64 },   /* player 1 */
	   { 740, 64 },   /* player 2 */
	   { 64,  600 },  /* player 3 */
	   { 740, 600 },  /* player 4 */
	};

	(void)hPrevInstance;
	(void)lpCmdLine;
	(void)nCmdShow;

	hwnd = CreateMainWindow(hInstance);

	memset(&wd, 0, sizeof(wd));
	WSAStartup(MAKEWORD(2, 2), &wd);

	if (__argc < 3) {
		Syntax();
		return 1;
	}
	local_port = (unsigned short)_wtoi(__wargv[offset++]);
	num_players = _wtoi(__wargv[offset++]);
	if (num_players < 0 || __argc < offset + num_players) {
		Syntax();
		return 1;
	}
	if (wcscmp(__wargv[offset], L"spectate") == 0) {
		char host_ip[128];
		unsigned short host_port;
		if (swscanf_s(__wargv[offset + 1], L"%[^:]:%hu", wide_ip_buffer, wide_ip_buffer_size, &host_port) != 2) {
			Syntax();
			return 1;
		}
		wcstombs_s(NULL, host_ip, ARRAYSIZE(host_ip), wide_ip_buffer, _TRUNCATE);
		VectorWar_InitSpectator(hwnd, local_port, num_players, host_ip, host_port);
	}
	else {
		GGPOPlayer players[GGPO_MAX_SPECTATORS + GGPO_MAX_PLAYERS];
		int i;
		int num_spectators = 0;

		for (i = 0; i < num_players; i++) {
			const wchar_t* arg = __wargv[offset++];

			players[i].size = sizeof(players[i]);
			players[i].player_num = i + 1;
			if (!_wcsicmp(arg, L"local")) {
				players[i].type = GGPO_PLAYERTYPE_LOCAL;
				local_player = i;
				continue;
			}

			players[i].type = GGPO_PLAYERTYPE_REMOTE;
#if defined(GGPO_STEAM)
			if (swscanf_s(arg, L"%llu", &players[i].u.steam_remote.steam_id) != 1) {
				Syntax();
				return 1;
			}
#else
			if (swscanf_s(arg, L"%[^:]:%hd", wide_ip_buffer, wide_ip_buffer_size, &players[i].u.remote.port) != 2) {
				Syntax();
				return 1;
			}
			wcstombs_s(NULL, players[i].u.remote.ip_address, ARRAYSIZE(players[i].u.remote.ip_address), wide_ip_buffer, _TRUNCATE);
#endif
		}
		/* these are spectators... */
		while (offset < __argc) {
			players[i].type = GGPO_PLAYERTYPE_SPECTATOR;

#if defined(GGPO_STEAM)
			if (swscanf_s(__wargv[offset++], L"%llu", &players[i].u.steam_remote.steam_id) != 1) {
				Syntax();
				return 1;
			}
#else
			if (swscanf_s(__wargv[offset++], L"%[^:]:%hd", wide_ip_buffer, wide_ip_buffer_size, &players[i].u.remote.port) != 2) {
				Syntax();
				return 1;
			}
			wcstombs_s(NULL, players[i].u.remote.ip_address, ARRAYSIZE(players[i].u.remote.ip_address), wide_ip_buffer, _TRUNCATE);
#endif
			i++;
			num_spectators++;
		}

		if (local_player < (int)(sizeof(window_offsets) / sizeof(window_offsets[0]))) {
			SetWindowPos(hwnd, NULL, window_offsets[local_player].x, window_offsets[local_player].y, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
		}

		VectorWar_Init(hwnd, local_port, num_players, players, num_spectators);
	}
	RunMainLoop(hwnd);
	VectorWar_Exit();
	WSACleanup();
	DestroyWindow(hwnd);
	return 0;
}
