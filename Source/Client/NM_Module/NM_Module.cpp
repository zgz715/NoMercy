#include "stdafx.h"
#include "../NM_ClientSDK/NoMercy.h"

#include "../../Client/NM_Engine/NM_Index.h"
#ifdef _DEBUG
	#pragma comment(lib, "../../Bin/Debug/NM_Engine.lib")
#else
	#pragma comment(lib, "../../Bin/Release/NM_Engine.lib")
#endif

static HMODULE s_hModule = nullptr;

inline bool CreateInfoData(PANTI_MODULE_INFO pami, HMODULE hModule)
{
#ifdef _M_X64
	auto pPEB = (PPEB)__readgsqword(0x60);
#elif _M_IX86
	auto pPEB = (PPEB)__readfsdword(0x30);
#else
	#error "architecture unsupported"
#endif

	auto CurrentEntry = pPEB->LoaderData->InLoadOrderModuleList.Flink;

	auto Current = PLDR_DATA_TABLE_ENTRY(NULL);
	while (CurrentEntry != &pPEB->LoaderData->InLoadOrderModuleList && CurrentEntry != NULL)
	{
		Current = CONTAINING_RECORD(CurrentEntry, LDR_DATA_TABLE_ENTRY, InLoadOrderLinks);
		if ((PVOID)hModule == Current->DllBase)
		{
			pami->BaseAddress = Current->DllBase;
			pami->EntryPoint = Current->EntryPoint;
			pami->SizeOfImage = Current->SizeOfImage;

			pami->BaseDllName.Buffer = Current->BaseDllName.Buffer;
			pami->BaseDllName.Length = Current->BaseDllName.Length;
			pami->BaseDllName.MaximumLength = Current->BaseDllName.MaximumLength;

			pami->FullDllName.Buffer = Current->FullDllName.Buffer;
			pami->FullDllName.Length = Current->FullDllName.Length;
			pami->FullDllName.MaximumLength = Current->FullDllName.MaximumLength;

			return true;
		}
		CurrentEntry = CurrentEntry->Flink;
	}
	return false;
}

#pragma optimize("", off)
extern "C" __declspec(dllexport) bool __cdecl Initialize(const char* c_szLicenseCode)
{
	if (!c_szLicenseCode || !*c_szLicenseCode || !strlen(c_szLicenseCode))
		return false;

	auto nmCommon = std::make_unique<CNoMercy>();
	if (!nmCommon || !nmCommon.get())
		return false;

	auto pModuleInfo = std::make_unique<ANTI_MODULE_INFO>();
	if (!pModuleInfo)
		return false;

	auto bCreateModuleInfo = CreateInfoData(pModuleInfo.get(), s_hModule);
	if (!bCreateModuleInfo)
		return false;

	bool bStandalone = false;
#ifdef _DEBUG
	auto hBaseModule = GetModuleHandleA(NULL);

	char pszName[MAX_PATH] = { 0 };
	GetModuleFileNameA(hBaseModule, pszName, _countof(pszName));

	std::string szName = pszName;
	std::transform(szName.begin(), szName.end(), szName.begin(), tolower);

	if (strstr(szName.c_str(), "nm_testapp.exe"))
		bStandalone = true;
#endif

	return nmCommon->InitCore(c_szLicenseCode, pModuleInfo.get(), bStandalone);
}

extern "C" __declspec(dllexport) bool __cdecl InitializeLauncher(const char* c_szLicenseCode, bool bProtected)
{
	auto nmCommon = std::make_unique<CNoMercy>();
	if (!nmCommon || !nmCommon.get())
		return false;

	return nmCommon->InitLauncher(c_szLicenseCode, bProtected);
}

extern "C" __declspec(dllexport) bool __cdecl InitializeService(bool bProtected)
{
	auto nmCommon = std::make_unique<CNoMercy>();
	if (!nmCommon || !nmCommon.get())
		return false;

	return nmCommon->InitService(bProtected);
}

extern "C" __declspec(dllexport) bool __cdecl InitializeShadow(HWND hwnd, HINSTANCE hinst, LPSTR lpszCmdLine, int nCmdShow)
{
	auto nmCommon = std::make_unique<CNoMercy>();
	if (!nmCommon || !nmCommon.get())
		return false;

	return nmCommon->InitShadow(s_hModule, hwnd, hinst, lpszCmdLine, nCmdShow);
}

extern "C" __declspec(dllexport) bool __cdecl InitializeHelper()
{
	auto nmCommon = std::make_unique<CNoMercy>();
	if (!nmCommon || !nmCommon.get())
		return false;

	return nmCommon->InitHelper();
}

extern "C" __declspec(dllexport) bool __cdecl Finalize()
{
	auto nmCommon = std::make_unique<CNoMercy>();
	if (!nmCommon || !nmCommon.get())
		return false;

	return nmCommon->Release();
}


extern "C" __declspec(dllexport) bool __cdecl SetupMsgHandler(TNMCallback lpMessageHandler)
{
	auto nmCommon = std::make_unique<CNoMercy>();
	if (!nmCommon || !nmCommon.get())
		return false;

	return nmCommon->CreateMessageHandler(lpMessageHandler);
}

extern "C" __declspec(dllexport) bool __cdecl MsgHelper(int Code, const void* lpMessage)
{
	auto nmCommon = std::make_unique<CNoMercy>();
	if (!nmCommon || !nmCommon.get())
		return false;

	return nmCommon->SendNMMessage(Code, lpMessage);
}
#pragma optimize("", on)


BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
	UNREFERENCED_PARAMETER(ul_reason_for_call);
	UNREFERENCED_PARAMETER(lpReserved);

	if (!s_hModule)
		s_hModule = hModule;

	return TRUE;
}

