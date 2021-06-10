#include "resolve.h"
#include <ntddk.h>

/// <summary>
/// Resolves the address to KeBugCheck2. The starting point of decompiling starts at
/// the address of KeBugCheckEx.
/// This works by searching for the call instruction and the next instruction being a NOP. 
/// There is only two instances of this, both leading to the call to KeBugCheck2. 
/// </summary>
/// <param name="data"></param>
/// <param name="result"></param>
/// <returns>STATUS_SUCCESS if successful</returns>
NTSTATUS resolve::KeBugCheck2(UINT64 address, PUINT64 result)
{
	UINT8 KeBugCheck2Sig[] = {
		0x45, 0x33, 0xc9,	/// xor r9d, r9d
		0x45, 0x33, 0xc0,	/// xor r8d, r8d
		0x33, 0xd2,			/// xor edx, edx
		0xe8				/// call nt!KeBugCheck2
	};

	UINT8 EndOfFunction[] = {
		0x90,	/// nop
		0xc3,	/// ret
		0xcc,	/// int 3
		0xcc,	/// int 3
		0xcc,	/// int 3
		0xcc	/// int 3
	};

	size_t EndOfFunctionLength = 0;

	do
	{
		size_t length = RtlCompareMemory(
			reinterpret_cast<PVOID>(address),
			KeBugCheck2Sig,
			sizeof(KeBugCheck2Sig)
		);
		if (length == sizeof(KeBugCheck2Sig))
		{
			UINT32 offset = 0;

			RtlCopyMemory(
				&offset, 
				reinterpret_cast<PVOID>(address + length), 
				sizeof(offset)
			);

			*result = address + length + offset + 4;

			return STATUS_SUCCESS;
		}

		EndOfFunctionLength = RtlCompareMemory(
			reinterpret_cast<PVOID>(address),
			EndOfFunction,
			sizeof(EndOfFunction)
		);

		address++;
	} while (EndOfFunctionLength != sizeof(EndOfFunction));
	
	return STATUS_NOT_FOUND;
}

/// <summary>
/// Resolves the address to KiDisplayBluescreen. This function is also not exported
/// and is easier to resolve via decompilation without having to rely on signature scanning.
/// This works by locating the instruction "cmovne ecx, eax" because the next instruction
/// is the call to nt!KiDisplayBlueScreen.
/// </summary>
/// <param name="data"></param>
/// <param name="result"></param>
/// <returns>STATUS_SUCCESS if successfull</returns>
NTSTATUS resolve::KiDisplayBlueScreen(UINT64 address, PUINT64 result)
{
	UINT8 KiDisplayBlueScreenSig[] = {
		0x0f, 0x45, 0xc1,	/// cmovne eax, ecx
		0x8b, 0xc8,			/// mov ecx, eax
		0x83, 0xc9, 0x01,	/// or ecx, 1
		0x45, 0x84, 0xf6,	/// test r14b, r14b
		0x0f, 0x45, 0xc8,	/// cmovne ecx, eax
		0xe8				/// call nt!KiDisplayBlueScreen
	};

	UINT8 EndOfFunction[] = {
		0x5f,	/// pop rdi
		0x5e,	/// pop rsi
		0x5d,	/// pop rbp
		0xc3	/// ret
	};

	size_t EndOfFunctionLength = 0;

	do
	{
		size_t length = RtlCompareMemory(
			reinterpret_cast<PVOID>(address),
			KiDisplayBlueScreenSig,
			sizeof(KiDisplayBlueScreenSig)
		);
		if (length == sizeof(KiDisplayBlueScreenSig))
		{
			UINT32 offset = 0;

			RtlCopyMemory(
				&offset,
				reinterpret_cast<PVOID>(address + length),
				sizeof(offset)
			);

			*result = address + length + offset + 4;

			return STATUS_SUCCESS;
		}

		EndOfFunctionLength = RtlCompareMemory(
			reinterpret_cast<PVOID>(address),
			EndOfFunction,
			sizeof(EndOfFunction)
		);

		address++;
	} while (EndOfFunctionLength != sizeof(EndOfFunction));

	return STATUS_NOT_FOUND;
}

/// <summary>
/// Resolves the address to BgpFwDisplayBugCheckScreen. This function is also not exported
/// and is easiest to resolve via decompilation. 
/// This works by locating the call instruction with the next instruction being a mov rdi, 
/// qword ptr [rsp + 78h]. There is only one instance of this sequence. The call is 
/// pointing to nt!BgpFwDisplayBugCheckScreen. 
/// </summary>
/// <param name="data"></param>
/// <param name="result"></param>
/// <returns>STATUS_SUCCESS if successful</returns>
NTSTATUS resolve::BgpFwDisplayBugCheckScreen(UINT64 address, PUINT64 result)
{
	UINT8 BgpFwDisplayBugCheckScreen2002Sig[] = {
		0x4c, 0x8b, 0xc3,	/// mov r8, rbx
		0x48, 0x8b, 0xd6,	/// mov rdx, rsi
		0x41, 0x8b, 0xcf,	/// mov ecx, r15d
		0xe8				/// call nt!BgpFwDisplayBugCheckScreen
	};

	UINT8 BgpFwDisplayBugCheckScreen1909Sig[] = {
		0x4c, 0x8b, 0xc3,	/// mov r8, rbx
		0x49, 0x8b, 0xd7,	/// mov rdx, r15
		0x41, 0x8b, 0xce,	/// mov ecx, r14d
		0xe8				/// call nt!BgpFwDisplayBugCheckScreen
	};

	UINT8 EndOfFunction[] = {
		0x41, 0x5d, /// pop r13
		0x41, 0x5c, /// pop r12
		0x5d,		/// pop rbp
		0xc3		/// ret
	};

	size_t EndOfFunctionLength = 0;
	size_t length = 0;
	UINT32 offset = 0;

	do
	{
		size_t Length2002 = RtlCompareMemory(
			reinterpret_cast<PVOID>(address),
			BgpFwDisplayBugCheckScreen2002Sig,
			sizeof(BgpFwDisplayBugCheckScreen2002Sig)
		);

		size_t Length1909 = RtlCompareMemory(
			reinterpret_cast<PVOID>(address),
			BgpFwDisplayBugCheckScreen1909Sig,
			sizeof(BgpFwDisplayBugCheckScreen1909Sig)
		);
		if (Length2002 == sizeof(BgpFwDisplayBugCheckScreen2002Sig))
		{
			length = Length2002;
			break;
		}
		else if (Length1909 == sizeof(BgpFwDisplayBugCheckScreen1909Sig))
		{
			length = Length1909;
			break;
		}

		EndOfFunctionLength = RtlCompareMemory(
			reinterpret_cast<PVOID>(address),
			EndOfFunction,
			sizeof(EndOfFunction)
		);
		if (EndOfFunctionLength == sizeof(EndOfFunction))
			return STATUS_NOT_FOUND;

		address++;
	} while (EndOfFunctionLength != sizeof(EndOfFunction));

	RtlCopyMemory(
		&offset,
		reinterpret_cast<PVOID>(address + length),
		sizeof(offset)
	);

	*result = address + length + offset + 4;

	return STATUS_SUCCESS;
}

/// <summary>
/// This function is responsible for resolving five addresses: the location for the LIST_ENTRY of
/// EtwpLastBranchEntry, the two offsets that are used to pull from this object, HalpPCIConfigReadHandlers,
/// and the location of the first error message. 
/// The two offsets are to avoid hardcoding the offsets for EtwpLastBranchEntry to resolve the color
/// of the Bluescreen of Death. The color is in ARGB format. EtwpLastBranchEntry is found by locating the 
/// instruction mov edi, 0x1c8. The instruction before this loads EtwpLastBranchEntry.
/// 
/// HalpPCIConfigReadHandlers is resolved because the value that is being loaded is actually a type
/// UNICODE_STRING pointing to the ":(". This is found by locating two instructions: mov r9d, ebx and 
/// lea rcx. There is only one sequence of this. The lea instruction loads HalpPCIConfigReadHandlers.
/// 
/// Lastly, we search for another EtwpLastBranchEntry only this time, it's the one responsible for the 
/// error messages that are displayed. Since all of the error messages related to the Bluescreen of Death
/// are in EtwpLastBranchEntry, there is no need to resolve the other locations as we can just iterate 
/// through EtwpLastBranchEntry with the sizeof(UNICODE_STRING). These are found by searching for the 
/// instruction lea r10, xxx. There is only one instance of this and loads exactly what I want. 
/// </summary>
/// <param name="data"></param>
/// <param name="halpPciConfig"></param>
/// <returns>STATUS_SUCCESS if successful</returns>
NTSTATUS resolve::Phrases(UINT64 address)//, PUINT64 halpPciConfig)
{
	UINT8 EtwpLastBranchSig[] = {
		0xbf, 0xc8, 0x01, 0x00, 0x00	/// mov edi, 1c8h
	};

	UINT8 SadfaceSig2004[] = {
		0x41, 0x8b, 0x54, 0xf7, 0x0c,	/// mov edx, dword ptr [r15 + rsi * 8 + 0ch]
		0x44, 0x8b, 0xcb,				/// mov r9d, ebx
		0x48, 0x8d						/// lea rcx, [nt!HalpPciConfigReadHandlers+0x18]
	};

	UINT8 SadfaceSig1909[] = {
		0x41, 0x8b, 0x54, 0xf4, 0x0c,	/// mov edx, dword ptr [r12 + rsi * 8 + 0ch]
		0x44, 0x8b, 0xcb,				/// mov r9d, ebx
		0x48, 0x8d						/// lea rcx, [nt!ExpLeapSecondRegKeyPath+0x28e0]
	};

	UINT8 ColorOffsetSig[] = {
		0xeb, 0x03,			/// jmp nt!BgpFwDiisplayBugCheckScreen+0xcd
		0x8b, 0x48, 0x28	/// mov ecx, dword ptr [rax + 28h]
	};

	UINT8 MessagesSig[] = {
		0x4c, 0x8d, 0x15	/// lea r10, [nt!EtwpLastBranchLookAsideList]
	};

	UINT8 EndOfFunction[] = {
		0x41, 0x5f, /// pop r15
		0x41, 0x5e, /// pop r14
		0x41, 0x5d, /// pop r13
		0x41, 0x5c, /// pop r12
	};

	bool bIsNotEndOfFunction = true;

	do
	{
		size_t EtwpLastBranchLength = RtlCompareMemory(
			reinterpret_cast<PVOID>(address + 7),
			EtwpLastBranchSig,
			sizeof(EtwpLastBranchSig)
		);

		size_t ColorOffsetLength = RtlCompareMemory(
			reinterpret_cast<PVOID>(address),
			ColorOffsetSig,
			sizeof(ColorOffsetSig)
		);

		size_t SadFaceLength2004 = RtlCompareMemory(
			reinterpret_cast<PVOID>(address),
			SadfaceSig2004,
			sizeof(SadfaceSig2004)
		);

		size_t SadFaceLength1909 = RtlCompareMemory(
			reinterpret_cast<PVOID>(address),
			SadfaceSig1909,
			sizeof(SadfaceSig1909)
		);

		size_t MessagesSigLength = RtlCompareMemory(
			reinterpret_cast<PVOID>(address),
			MessagesSig,
			sizeof(MessagesSig)
		);

		size_t EndOfFunctionLength = RtlCompareMemory(
			reinterpret_cast<PVOID>(address),
			EndOfFunction,
			sizeof(EndOfFunction)
		);
		if (EndOfFunctionLength == sizeof(EndOfFunction))
		{
			bIsNotEndOfFunction = false;
		}
		else if (MessagesSigLength == sizeof(MessagesSig))
		{
			UINT64 TempAddress = address;
			UINT32 offset = 0;

			RtlCopyMemory(
				&offset,
				reinterpret_cast<PVOID>(TempAddress + MessagesSigLength),
				sizeof(offset)
			);

			g_BsodInformation->BsodMessageOne = reinterpret_cast<PUNICODE_STRING>(
				TempAddress + MessagesSigLength + offset + 4
				);

			PUNICODE_STRING temp = g_BsodInformation->BsodMessageOne;

			for (UINT8 next = 0; next < sizeof(UNICODE_STRING); next++, temp++)
			{
				/*
				*[FFFFF8010F853CD0] Got: Have you seen your skills? I ain't worried one bit.
				*[FFFFF8010F853CE0] Got: If you call a support person, give them this info:
				*[FFFFF8010F853CF0] Got: We're just collecting some error info, and then we'll restart for you.
				*[FFFFF8010F853D00] Got: We're just collecting some error info, and then you can restart.
				*[FFFFF8010F853D10] Got: We'll restart for you.
				*[FFFFF8010F853D20] Got: You can restart.
				*[FFFFF8010F853D30] Got:
				*[FFFFF8010F853D40] Got: % complete
				*[FFFFF8010F853D50] Got:
				*[FFFFF8010F853D60] Got: % complete
				*[FFFFF8010F853D70] Got: What failed:
				*[FFFFF8010F853D80] Got: Stop code:
				*[FFFFF8010F853D90] Got: For more information about this issue and possible fixes, visit
				*[FFFFF8010F853DA0] Got: https://www.windows.com/stopcode
				*[FFFFF8010F853DB0] Got: Your Windows Insider Build ran into a problem and needs to restart.
				*[FFFFF8010F853DC0] Got: Please release the power button.
				*/
				if (wcsstr(temp->Buffer, L"and then we'll restart for you") != 0)
				{
					g_BsodInformation->BsodMessageTwo = temp;
				}
			}
		}
		else if (EtwpLastBranchLength == sizeof(EtwpLastBranchSig))
		{
			UINT64 TempAddress = address;
			UINT32 offset = 0;

			RtlCopyMemory(
				&g_BsodInformation->offset,
				reinterpret_cast<PVOID>(TempAddress + 0x12),
				sizeof(g_BsodInformation->offset)
			);

			TempAddress += 3;

			RtlCopyMemory(
				&offset,
				reinterpret_cast<PVOID>(TempAddress),
				sizeof(offset)
			);

			TempAddress += offset + 4;

			g_BsodInformation->EtwpLastBranchEntry = TempAddress;
		}
		else if (ColorOffsetLength == sizeof(ColorOffsetSig))
		{			
			RtlCopyMemory(
				&g_BsodInformation->colorOffset,
				reinterpret_cast<PVOID>(address + 4),
				sizeof(g_BsodInformation->colorOffset)
			);
		}
		else if (SadFaceLength2004 == sizeof(SadfaceSig2004))
		{
			UINT64 TempAddress = address;
			UINT64 mask = 0xffffffff00000000;
			UINT32 offset = 0;

			TempAddress += SadFaceLength2004 + 1;
			
			RtlCopyMemory(
				&offset,
				reinterpret_cast<PVOID>(TempAddress),
				sizeof(offset)
			);

			mask |= offset;

			TempAddress += mask + 4;

			g_BsodInformation->Sadface = TempAddress;
		}
		else if (SadFaceLength1909 == sizeof(SadfaceSig1909))
		{
			UINT64 TempAddress = address;
			UINT32 offset = 0;

			TempAddress += SadFaceLength1909 + 1;

			RtlCopyMemory(
				&offset,
				reinterpret_cast<PVOID>(TempAddress),
				sizeof(offset)
			);

			TempAddress += offset + 4;

			g_BsodInformation->Sadface = TempAddress;
		}

		address++;
	} while (bIsNotEndOfFunction);

	return STATUS_SUCCESS;
}


/// EOF