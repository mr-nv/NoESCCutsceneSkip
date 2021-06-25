#pragma once
#include <Windows.h>
#include <string>
#include <algorithm>

// minhook
#include "minhook/MinHook.h"

// util
namespace util
{
	inline DWORD FindPattern( void* module, std::string pattern )
	{
		auto header = ( ( PIMAGE_NT_HEADERS )( ( DWORD )module + ( ( PIMAGE_DOS_HEADER )module )->e_lfanew ) )->OptionalHeader;
		auto end = ( byte* )module + header.SizeOfCode;

		auto ret = std::search( ( byte* )module, end, pattern.begin( ), pattern.end( ), [ ]( const byte& a, const char& b )
		{
			return ( a == ( byte )b || b == '?' );
		} );

		if( ret >= end )
			return 0;

		return ( DWORD )ret;
	}

	inline DWORD GetAddressFromCall( DWORD address )
	{
		return ( ( *( DWORD* )( address + 1 ) + address + 5 ) );
	}
}