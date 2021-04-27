#pragma once
#include <ntddk.h>
#include "resolve.h"

/// <summary>
/// The namespace responsible for modifying the Bluescreen of
/// Death. This will only modify the sad face emoticon and the
/// first error message. 
/// </summary>
namespace bluescreen
{
	NTSTATUS initialize();
	NTSTATUS OverwriteSadFace();

	NTSTATUS OverwriteErrorMessage(
		_In_ PUNICODE_STRING error,
		_In_ PUNICODE_STRING replace
	);

	NTSTATUS ChangeBsodColor(
		_In_ UINT32 color
	);
}


/// EOF 