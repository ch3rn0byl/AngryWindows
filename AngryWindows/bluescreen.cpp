#include "bluescreen.h"

/// <summary>
/// This will dynamically resolve all the variables needed to be able 
/// to modify the Bluescreen of Death. If any of the variables are not 
/// able to be found, STATUS_NOT_FOUND will be returned. 
/// </summary>
/// <returns>STATUS_SUCCESS/STATUS_NOT_FOUND</returns>
NTSTATUS bluescreen::initialize()
{
	UNICODE_STRING SystemRoutineName = RTL_CONSTANT_STRING(L"KeBugCheckEx");

	UINT64 KeBugCheck2 = 0;
	UINT64 KiDisplayBlueScreen = 0;
	UINT64 BgpFwDisplayBugCheckScreen = 0;

	PVOID KeBugCheckExAddress = MmGetSystemRoutineAddress(&SystemRoutineName);
	if (KeBugCheckExAddress == NULL)
	{
		DbgPrint("[%ws::%d] Unable to resolve KeBugCheckEx\n", __FUNCTIONW__, __LINE__);
		return STATUS_NOT_FOUND;
	}

	cs_err csStatus = resolve::KeBugCheck2(KeBugCheckExAddress, &KeBugCheck2);
	if (csStatus != CS_ERR_OK || KeBugCheck2 == 0)
	{
		DbgPrint("[%ws::%d] Unable to resolve KeBugCheck2\n", __FUNCTIONW__, __LINE__);
		return STATUS_NOT_FOUND;
	}

	csStatus = resolve::KiDisplayBlueScreen(
		reinterpret_cast<PVOID>(KeBugCheck2),
		&KiDisplayBlueScreen
	);
	if (csStatus != CS_ERR_OK || KiDisplayBlueScreen == 0)
	{
		DbgPrint("[%ws::%d] Unable to resolve KiDisplayBlueScreen\n", __FUNCTIONW__, __LINE__);
		return STATUS_NOT_FOUND;
	}

	csStatus = resolve::BgpFwDisplayBugCheckScreen(
		reinterpret_cast<PVOID>(KiDisplayBlueScreen),
		&BgpFwDisplayBugCheckScreen
	);
	if (csStatus != CS_ERR_OK || BgpFwDisplayBugCheckScreen == 0)
	{
		DbgPrint("[%ws::%d] Unable to resolve BgpFwDisplayBugCheckScreen\n", __FUNCTIONW__, __LINE__);
		return STATUS_NOT_FOUND;
	}

	csStatus = resolve::HalpPCIConfigReadHandlers(
		reinterpret_cast<PVOID>(BgpFwDisplayBugCheckScreen),
		&g_BsodInformation->HalpPCIConfigReadHandlers
	);
	if (csStatus != CS_ERR_OK || g_BsodInformation->HalpPCIConfigReadHandlers == 0)
	{
		DbgPrint("[%ws::%d] Unable to resolve HalpPCIConfigReadHandlers\n", __FUNCTIONW__, __LINE__);
		return STATUS_NOT_FOUND;
	}

	return STATUS_SUCCESS;
}

/// <summary>
/// The location of the sad emoticon is in read only memory. To modify the 
/// permissions of this location, the physical address is mapped as ReadWrite and then written to. 
/// This is done twice to adjust the size of the UNICODE_STRING, and then the buffer
/// itself is mapped as ReadWrite to be able to modify the contents of the buffer. 
/// The mapped page is then unmapped, and CR0.WP is left alone. 
/// </summary>
/// <returns>STATUS_SUCCESS/STATUS_INSUFFICIENT_RESOURCES</returns>
NTSTATUS bluescreen::OverwriteSadFace()
{
	PHYSICAL_ADDRESS pa = MmGetPhysicalAddress(
		reinterpret_cast<PVOID>(g_BsodInformation->HalpPCIConfigReadHandlers)
	);

	PUNICODE_STRING mappedAddress = static_cast<PUNICODE_STRING>(
		MmMapIoSpace(pa, sizeof(UNICODE_STRING), MmNonCached)
		);
	if (mappedAddress == NULL)
	{
		DbgPrint("[%ws::%d] MmMapIoSpace failed: %08x\n", __FUNCTIONW__, __LINE__, STATUS_INSUFFICIENT_RESOURCES);
		return STATUS_INSUFFICIENT_RESOURCES;
	}

	mappedAddress->Length = g_BsodInformation->lol.Length;
	mappedAddress->MaximumLength = g_BsodInformation->lol.MaximumLength;

	PHYSICAL_ADDRESS paBuffer = MmGetPhysicalAddress(mappedAddress->Buffer);

	PVOID mappedBuffer = MmMapIoSpace(paBuffer, mappedAddress->MaximumLength, MmNonCached);
	if (mappedBuffer == NULL)
	{
		MmUnmapIoSpace(mappedAddress, sizeof(UNICODE_STRING));
		DbgPrint("[%ws::%d] MmMapIoSpace failed: %08x\n", __FUNCTIONW__, __LINE__, STATUS_INSUFFICIENT_RESOURCES);
		return STATUS_INSUFFICIENT_RESOURCES;
	}

	RtlCopyMemory(mappedBuffer, g_BsodInformation->lol.Buffer, g_BsodInformation->lol.Length);

	MmUnmapIoSpace(mappedBuffer, mappedAddress->MaximumLength);
	MmUnmapIoSpace(mappedAddress, sizeof(UNICODE_STRING));

	return STATUS_SUCCESS;
}

/// <summary>
/// The same principal applies for the location of the error message. The location it lies at is
/// read only. The same steps for modifying the sad face emoticon applies here as well. 
/// </summary>
/// <returns>STATUS_SUCCESS/STATUS_INSUFFICIENT_RESOURCES</returns>
NTSTATUS bluescreen::OverwriteErrorMessage(PUNICODE_STRING error, PUNICODE_STRING replace)
{
	PHYSICAL_ADDRESS pa = MmGetPhysicalAddress(
		reinterpret_cast<PVOID>(error)
	);

	PUNICODE_STRING mappedAddress = reinterpret_cast<PUNICODE_STRING>(
		MmMapIoSpace(pa, sizeof(UNICODE_STRING), MmNonCached)
		);
	if (mappedAddress == NULL)
	{
		DbgPrint("[%ws::%d] MmMapIoSpace failed: %08x\n", __FUNCTIONW__, __LINE__, STATUS_INSUFFICIENT_RESOURCES);
		return STATUS_INSUFFICIENT_RESOURCES;
	}

	mappedAddress->Length = replace->Length;
	mappedAddress->MaximumLength = replace->MaximumLength;

	PHYSICAL_ADDRESS paBuffer = MmGetPhysicalAddress(mappedAddress->Buffer);

	PVOID mappedBuffer = MmMapIoSpace(paBuffer, mappedAddress->MaximumLength, MmNonCached);
	if (mappedBuffer == NULL)
	{
		MmUnmapIoSpace(mappedAddress, sizeof(UNICODE_STRING));

		DbgPrint("[%ws::%d] MmMapIoSpace failed: %08x\n", __FUNCTIONW__, __LINE__, STATUS_INSUFFICIENT_RESOURCES);
		return STATUS_INSUFFICIENT_RESOURCES;
	}

	RtlCopyMemory(mappedBuffer, replace->Buffer, replace->Length);

	MmUnmapIoSpace(mappedBuffer, mappedAddress->MaximumLength);
	MmUnmapIoSpace(mappedAddress, sizeof(UNICODE_STRING));

	return STATUS_SUCCESS;
}

/// <summary>
/// The color it takes in to change the Bluescreen of Death is of type UINT32 and in
/// ARGB format. There is no need to modify the permissions of the page as it is 
/// read/write. 
/// </summary>
/// <param name="color"></param>
/// <returns>STATUS_SUCCESS</returns>
NTSTATUS bluescreen::ChangeBsodColor(UINT32 color)
{
	RtlCopyMemory(
		&g_BsodInformation->AddressOfColorVar,
		reinterpret_cast<PVOID>(g_BsodInformation->EtwpLastBranchEntry),
		sizeof(g_BsodInformation->EtwpLastBranchEntry)
	);

	g_BsodInformation->AddressOfColorVar += g_BsodInformation->offset;

	RtlCopyMemory(
		&g_BsodInformation->AddressOfColorVar,
		reinterpret_cast<PVOID>(g_BsodInformation->AddressOfColorVar),
		sizeof(g_BsodInformation->AddressOfColorVar)
	);

	g_BsodInformation->AddressOfColorVar += g_BsodInformation->colorOffset;

	*(PUINT32)g_BsodInformation->AddressOfColorVar = color;

	return STATUS_SUCCESS;
}


/// EOF