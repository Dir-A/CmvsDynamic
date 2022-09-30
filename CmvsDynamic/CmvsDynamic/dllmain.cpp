#include <Windows.h>
#include <detours.h>
#include <iostream>
#include <vector>
#include "Tools.h"

std::string g_strFileName;
std::vector<PDWORD> g_vlpTHIS;
std::vector<std::string> g_vstrPackPath;
PDWORD g_lpVtable = 0;
LPCSTR g_lpString = 0;
BOOL g_isInit = FALSE;

typedef HLOCAL(__thiscall* pReadFile)(
	PDWORD tThis,
	LPCSTR lpPackPath,
	LPCSTR lpString,
	LPCSTR lpFileName,
	PDWORD lpSize);
pReadFile rawReadFile;

typedef DWORD(__thiscall* pGetPackInfo)(
	PDWORD pTHIS,
	DWORD dwUnKnow0,
	DWORD dwUnKnow1);
pGetPackInfo rawGetPackInfo = (pGetPackInfo)0x004100F0;

BOOL ExtractPack()
{
	DWORD size = 0;
	PDWORD hlocal = 0;
	HANDLE hFile = 0;
	size_t number = 0;
	size_t i = 0;

	if (g_vstrPackPath.size() == g_vlpTHIS.size())
	{
		for (; i < g_vstrPackPath.size(); i++)
		{
			std::cout << "M ->" << i << "<- " << "PackPath:" << g_vstrPackPath[i] << std::endl;
		}
	}

	while (true)
	{
		std::cout << "PackNumber:";
		std::cin >> number;
		std::cout << std::endl;
		std::cout << "FileName::";
		std::cin >> g_strFileName;
		std::cout << std::endl;

		if ((number <= i) && (!g_strFileName.empty()))
		{
			hlocal = (PDWORD)rawReadFile(g_vlpTHIS[number], g_vstrPackPath[number].c_str(), g_lpString, g_strFileName.c_str(), &size);
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
				else
				{
					std::cout << "CreateFile failed!!!" << std::endl;
				}
				LocalFree(hlocal);
			}
			else
			{
				std::cout << "ReadFile failed!!!" << std::endl;
			}

		}
	}
}

HLOCAL __fastcall newReadFile(DWORD* tThis, DWORD dwReserved, LPCSTR lpPackPath, LPCSTR lpString, LPCSTR lpFileName, PDWORD lpSize)
{
	if (!g_isInit)
	{
		g_lpString = lpString;
		if (g_lpString != NULL)
		{
			g_isInit = TRUE;
			CreateThread(NULL, NULL, (LPTHREAD_START_ROUTINE)ExtractPack, NULL, NULL, NULL);
		}
	}

	return rawReadFile(tThis, lpPackPath, lpString, lpFileName, lpSize);
}

DWORD __fastcall newGetPackInfo(PDWORD pTHIS, DWORD dwReserved, DWORD dwUnKnow0, DWORD dwUnKnow1)
{
	if (!g_lpVtable)
	{
		g_lpVtable = (PDWORD)*pTHIS;
		rawReadFile = (pReadFile)g_lpVtable[3];
		DetourTransactionBegin();
		DetourUpdateThread(GetCurrentThread());
		DetourAttach(&(PVOID&)rawReadFile, newReadFile);
		DetourTransactionCommit();
	}

	if (!g_isInit)
	{
		PDWORD pPath = &dwUnKnow1 + 0x2;
		std::string path = (LPCSTR)*pPath;
		if (path.find(".cpz") != std::string::npos)
		{
			g_vstrPackPath.push_back(path);
			g_vlpTHIS.push_back(pTHIS);
		}
	}

	return rawGetPackInfo(pTHIS, dwUnKnow0, dwUnKnow1);
}

VOID StartHook()
{
	SetConsole();

	DetourRestoreAfterWith();
	DetourTransactionBegin();
	DetourUpdateThread(GetCurrentThread());
	DetourAttach(&(PVOID&)rawGetPackInfo, newGetPackInfo);
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