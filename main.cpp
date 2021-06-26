#include "main.h"

typedef bool( __cdecl* IsCutsceneSkipButtonBeingPressedFn )( );
IsCutsceneSkipButtonBeingPressedFn original_IsCutsceneSkipButtonBeingPressed;

WNDPROC original_WindowProc;

bool pressedenter = false;

bool __cdecl hooked_IsCutsceneSkipButtonBeingPressed( )
{
	static auto gamewindow = FindWindowA( 0, "GTA: San Andreas" );
	if( !gamewindow )
	{
		// cant remember if this works.............
		static bool once = ( true, MessageBoxA( 0, "FindWindowA failed", "NoESCCutsceneSkip", MB_OK ) );
		return original_IsCutsceneSkipButtonBeingPressed( );
	}

	if( GetActiveWindow( ) != gamewindow ) return original_IsCutsceneSkipButtonBeingPressed( );

	return pressedenter;
}

LRESULT __stdcall hooked_WindowProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	switch( uMsg )
	{
		case WM_KEYUP:
			if( wParam == VK_RETURN )
				pressedenter = false;

			break;
		case WM_KEYDOWN:
			if( wParam == VK_RETURN )
				pressedenter = true;

			break;
		default: break;
	}

	return CallWindowProcW( original_WindowProc, hWnd, uMsg, wParam, lParam );
}

void Start( )
{
	DWORD address = util::FindPattern( ( void* )GetModuleHandleA( 0 ), "\xE8????\x84\xC0\x74\x38" );

	while( !address )
	{
		address = util::FindPattern( ( void* )GetModuleHandleA( 0 ), "\xE8????\x84\xC0\x74\x38" );
		Sleep( 2000 );
	}

	address = util::GetAddressFromCall( address );

	auto init = MH_Initialize( );
	if( init != MH_OK )
	{
		char ch[ 1024 ];
		sprintf_s( ch, "MH_Initialize failed with status %s", MH_StatusToString( init ) );

		MessageBoxA( 0, ch, "NoESCCutsceneSkip", MB_OK );
		return;
	}

	auto createhook = MH_CreateHook( ( void* )address, &hooked_IsCutsceneSkipButtonBeingPressed,
		( void** )original_IsCutsceneSkipButtonBeingPressed );
	if( createhook != MH_OK )
	{
		char ch[ 1024 ];
		sprintf_s( ch, "MH_CreateHook failed with status %s", MH_StatusToString( createhook ) );

		MessageBoxA( 0, ch, "NoESCCutsceneSkip", MB_OK );
		return;
	}

	// hook windowproc
	HWND gamewindow = nullptr;
	while( !gamewindow )
	{
		gamewindow = FindWindowA( 0, "GTA: San Andreas" );
		Sleep( 1000 );
	}

	original_WindowProc = ( WNDPROC )SetWindowLongPtr( gamewindow, GWL_WNDPROC, ( LONG_PTR )hooked_WindowProc );

	auto enablehook = MH_EnableHook( MH_ALL_HOOKS );
	if( enablehook != MH_OK )
	{
		char ch[ 1024 ];
		sprintf_s( ch, "MH_EnableHook failed with status %s", MH_StatusToString( enablehook ) );

		MessageBoxA( 0, ch, "NoESCCutsceneSkip", MB_OK );
		return;
	}
}

int WINAPI DllMain( HINSTANCE instance, DWORD reason, LPVOID reserved )
{
	if( reason == DLL_PROCESS_ATTACH )
	{
		const auto thread = CreateThread( 0, 0, ( LPTHREAD_START_ROUTINE )Start, reserved, 0, 0 );
		if( thread )
			CloseHandle( ( HANDLE )thread );
	}

	return TRUE;
}
