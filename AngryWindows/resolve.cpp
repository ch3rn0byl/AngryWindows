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
cs_err resolve::KeBugCheck2(PVOID data, uint64_t* result)
{
	csh handle;
	cs_insn* insn = nullptr;

	cs_err csStatus = cs_open(CS_ARCH_X86, CS_MODE_64, &handle);
	if (csStatus != CS_ERR_OK)
	{
		return csStatus;
	}

	size_t NumberOfInsts = cs_disasm(
		handle,
		reinterpret_cast<const uint8_t*>(data),
		0x150,
		reinterpret_cast<uint64_t>(data),
		0,
		&insn
	);
	if (NumberOfInsts == 0)
	{
		csStatus = cs_errno(handle);
		cs_close(&handle);

		return csStatus;
	}

	for (size_t i = 0; i < NumberOfInsts; i++)
	{
		if (strcmp(insn[i].mnemonic, "call") == 0 && strcmp(insn[i + 1].mnemonic, "nop") == 0)
		{
			*result = _strtoui64(insn[i].op_str, NULL, 16);
			break;
		}
	}

	cs_free(insn, NumberOfInsts);
	cs_close(&handle);

	return csStatus;
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
cs_err resolve::KiDisplayBlueScreen(PVOID data, uint64_t* result)
{
	csh handle;
	cs_insn* insn = nullptr;

	cs_err csStatus = cs_open(CS_ARCH_X86, CS_MODE_64, &handle);
	if (csStatus != CS_ERR_OK)
	{
		return csStatus;
	}

	size_t NumberOfInsts = cs_disasm(
		handle,
		reinterpret_cast<const uint8_t*>(data),
		0xb00,
		reinterpret_cast<uint64_t>(data),
		0,
		&insn
	);
	if (NumberOfInsts == 0)
	{
		csStatus = cs_errno(handle);
		cs_close(&handle);

		return csStatus;
	}

	for (size_t i = 0; i < NumberOfInsts; i++)
	{
		if (strcmp(insn[i].mnemonic, "cmovne") == 0 && strcmp(insn[i].op_str, "ecx, eax") == 0)
		{
			*result = _strtoui64(insn[i + 1].op_str, NULL, 16);
			break;
		}
	}

	cs_free(insn, NumberOfInsts);
	cs_close(&handle);

	return csStatus;
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
cs_err resolve::BgpFwDisplayBugCheckScreen(PVOID data, uint64_t* result)
{
	csh handle;
	cs_insn* insn = nullptr;

	cs_err csStatus = cs_open(CS_ARCH_X86, CS_MODE_64, &handle);
	if (csStatus != CS_ERR_OK)
	{
		return csStatus;
	}

	size_t NumberOfInsts = cs_disasm(
		handle,
		reinterpret_cast<const uint8_t*>(data),
		0x250,
		reinterpret_cast<uint64_t>(data),
		0,
		&insn
	);
	if (NumberOfInsts == 0)
	{
		csStatus = cs_errno(handle);
		cs_close(&handle);

		return csStatus;
	}

	for (size_t i = 0; i < NumberOfInsts; i++)
	{
		if (strcmp(insn[i].mnemonic, "call") == 0 && strstr(insn[i + 1].op_str, "rdi, qword ptr") != 0)
		{
			*result = _strtoui64(insn[i].op_str, NULL, 16);
			break;
		}
	}

	cs_free(insn, NumberOfInsts);
	cs_close(&handle);

	return csStatus;
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
cs_err resolve::HalpPCIConfigReadHandlers(PVOID data, uint64_t* halpPciConfig)
{
	csh handle;
	cs_insn* insn = nullptr;

	cs_err csStatus = cs_open(CS_ARCH_X86, CS_MODE_64, &handle);
	if (csStatus != CS_ERR_OK)
	{
		return csStatus;
	}

	size_t NumberOfInsts = cs_disasm(
		handle,
		reinterpret_cast<const uint8_t*>(data),
		0x250,
		reinterpret_cast<uint64_t>(data),
		0,
		&insn
	);
	if (NumberOfInsts == 0)
	{
		csStatus = cs_errno(handle);
		cs_close(&handle);

		return csStatus;
	}

	for (size_t i = 0; i < NumberOfInsts; i++)
	{
		if (strcmp(insn[i + 1].mnemonic, "mov") == 0 && strcmp(insn[i + 1].op_str, "edi, 0x1c8") == 0)
		{
			UINT32 offset = 0;
			RtlCopyMemory(&offset, &insn[i].bytes[3], sizeof(UINT32));

			offset += insn[i].size;

			g_BsodInformation->EtwpLastBranchEntry = insn[i].address + offset;
		}
		else if (strcmp(insn[i].mnemonic, "mov") == 0 && strstr(insn[i].op_str, "rax, qword ptr") != 0)
		{
			g_BsodInformation->offset = insn[i].bytes[insn[i].size - 1];
		}
		else if (strcmp(insn[i].mnemonic, "mov") == 0 && strstr(insn[i].op_str, "ecx, dword ptr [rax + ") != 0)
		{
			g_BsodInformation->colorOffset = insn[i].bytes[insn[i].size - 1];
		}
		else if (strcmp(insn[i].mnemonic, "mov") == 0 &&
			strcmp(insn[i].op_str, "r9d, ebx") == 0 &&
			strcmp(insn[i + 1].mnemonic, "lea") == 0)
		{
			UINT64 mask = 0xffffffff00000000;
			UINT32 offset = 0;

			RtlCopyMemory(&offset, &insn[i + 1].bytes[3], sizeof(UINT32));

			UINT64 Result = insn[i + 1].address;
			Result += mask | offset;
			Result -= 1;
			Result += 8;

			*halpPciConfig = Result;
		}
		else if (strcmp(insn[i].mnemonic, "lea") == 0 && strstr(insn[i].op_str, "r10") != 0)
		{
			UINT32 offset = 0;
			RtlCopyMemory(&offset, &insn[i].bytes[3], sizeof(UINT32));

			offset += insn[i].size;

			g_BsodInformation->BsodMessageOne = reinterpret_cast<PUNICODE_STRING>(
				insn[i].address + offset
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

			break;
		}
	}

	cs_free(insn, NumberOfInsts);
	cs_close(&handle);

	return csStatus;
}


/// EOF