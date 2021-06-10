#pragma once
//#include "capstone/capstone.h"
#include "typedefs.h"

#include <ntdef.h>

/// <summary>
/// This namespace is responsible for resolving all the variables needed
/// for modifying the Bluescreen of Death. It dynamically locates
/// KeBugCheck2, KiDisplayBluescreen, BgpFwDisplayBugCheckScreen,
/// HalpPCIConfigReadHandlers, the sad face emoticon, and one of the error
/// messages. 
/// </summary>
namespace resolve
{
	_Success_(return >= 0)
	NTSTATUS KeBugCheck2(
		_In_ UINT64 address,
		_Out_ _Ret_maybenull_ PUINT64 result
	);

	_Success_(return >= 0)
	NTSTATUS KiDisplayBlueScreen(
		_In_ UINT64 address,
		_Out_ _Ret_maybenull_ PUINT64 result
	);

	_Success_(return >= 0)
	NTSTATUS BgpFwDisplayBugCheckScreen(
		_In_ UINT64 address,
		_Out_ _Ret_maybenull_ PUINT64 result
	);

	_Success_(return >= 0)
	NTSTATUS Phrases(
		_In_ UINT64 address
		//_Out_ _Ret_maybenull_ PUINT64 halpPciConfig
	);
}


/// EOF