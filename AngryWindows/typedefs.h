#pragma once
#include <ntdef.h>

///================================================
/// Types
///================================================
typedef struct _BSOD_INFORMATION
{
	UINT64 HalpPCIConfigReadHandlers = 0;
	UINT64 EtwpLastBranchEntry = 0;
	UINT64 AddressOfColorVar = 0;
	UINT8 offset = 0;
	UINT8 colorOffset = 0;
	UNICODE_STRING lol;
	UNICODE_STRING Grouch;
	UNICODE_STRING Insult;
	PUNICODE_STRING BsodMessageOne;
	PUNICODE_STRING BsodMessageTwo;
} BSOD_INFORMATION, * PBSOD_INFORMATION;

///================================================
/// Globals
///================================================
EXTERN_C int _fltused;
extern PBSOD_INFORMATION g_BsodInformation;


/// EOF 