#include <Windows.h>
#include <detours.h>
#include <iostream>
#include "Tools.h"

std::string g_strPackPath = ".\\data\\pack\\";
std::string g_strPackName = "script.cpz";
std::string g_strFileName;
PDWORD g_lpUnKnow = 0;

typedef HLOCAL(__thiscall* pReadFile)(
	DWORD* tThis,
	LPCSTR Path,
	DWORD* lpUnKnow,
	LPCSTR lpFileName,
	DWORD* lpSize);
pReadFile rawReadFile = (pReadFile)0x00410350;

typedef DWORD(__thiscall* pRegPack)(
	DWORD* pTHIS,
	DWORD dwUnKnow,
	DWORD dwUnKnow1);
pRegPack rawRegPack = (pRegPack)0x004100F0;

VOID Dump()
{
	DWORD size = 0;
	PDWORD hlocal = 0;
	HANDLE hFile = 0;
	DWORD pTHIS = 0;

	while (true)
	{
		std::cout << std::endl;
		std::cout << "InputFileNmae:";
		std::cin >> g_strFileName;
		std::cout << std::endl;
		std::cout << "PackName:";
		std::cin >> g_strPackName;
		std::cout << std::endl;
		std::cout << "THIS:";
		std::cin >> std::hex >> pTHIS;

		if (g_lpUnKnow)
		{
			hlocal = (PDWORD)rawReadFile((PDWORD)pTHIS, (g_strPackPath + g_strPackName).c_str(), g_lpUnKnow, g_strFileName.c_str(), &size);
			if (hlocal != NULL)
			{
				hFile = CreateFileA(g_strFileName.c_str(), GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
				if (hFile != INVALID_HANDLE_VALUE)
				{
					WriteFile(hFile, hlocal, size, NULL, NULL);
					FlushFileBuffers(hFile);
					CloseHandle(hFile);
					std::cout << "Dump:" << g_strFileName << std::endl;
				}
				LocalFree(hlocal);
			}

		}
	}
}

HLOCAL __fastcall newReadFile(DWORD* tThis, DWORD dwReserved, LPCSTR lpPath, DWORD* lpUnKnow, LPCSTR lpFileName, DWORD* lpSize)
{
	if (g_lpUnKnow == NULL)
	{
		CreateThread(NULL, NULL, (LPTHREAD_START_ROUTINE)Dump, NULL, NULL, NULL);
	}
	g_lpUnKnow = lpUnKnow;

	return rawReadFile(tThis, lpPath, lpUnKnow, lpFileName, lpSize);
}

DWORD __fastcall newRegPack(DWORD* pTHIS, DWORD dwReserved, DWORD dwUnKnow, DWORD dwUnKnow1)
{
	PDWORD pPath = &dwUnKnow1 + 0x2;
	std::string path = (LPCSTR)*pPath;
	if (path.find(".cpz") != std::string::npos)
	{
		std::cout << "Path:" << path << std::endl;
		std::cout << "Addr:0x" << pTHIS << std::endl;
	}

	return rawRegPack(pTHIS, dwUnKnow, dwUnKnow1);
}

VOID StartHook()
{
	SetConsole();

	DetourRestoreAfterWith();
	DetourTransactionBegin();
	DetourUpdateThread(GetCurrentThread());
	DetourAttach(&(PVOID&)rawReadFile, newReadFile);
	DetourAttach(&(PVOID&)rawRegPack, newRegPack);
	DetourTransactionCommit();
}

BOOL APIENTRY DllMain(HMODULE hModule,
	DWORD  ul_reason_for_call,
	LPVOID lpReserved
)
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		StartHook();
		break;
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
	case DLL_PROCESS_DETACH:
		break;
	}
	return TRUE;
}

VOID __declspec(dllexport) DirA() {}