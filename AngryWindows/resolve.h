#pragma once
#include "capstone/capstone.h"
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
	cs_err KeBugCheck2(
		_In_ PVOID data,
		_Out_ uint64_t* result
	);

	cs_err KiDisplayBlueScreen(
		_In_ PVOID data,
		_Out_ uint64_t* result
	);

	cs_err BgpFwDisplayBugCheckScreen(
		_In_ PVOID data,
		_Out_ uint64_t* result
	);

	cs_err HalpPCIConfigReadHandlers(
		_In_ PVOID data,
		_Out_ uint64_t* halpPciConfig
	);
}


/// EOF