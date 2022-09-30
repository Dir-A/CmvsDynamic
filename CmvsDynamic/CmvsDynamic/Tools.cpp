#include "Tools.h"

DWORD WINAPI SetConsoleTop(LPVOID lpParameter)
{
	WCHAR consoleTitle[MAX_PATH] = { 0 };

	while (true)
	{
		GetConsoleTitleW(consoleTitle, 256);
		HWND hConsole = FindWindowW(NULL, (LPWSTR)consoleTitle);
		if (hConsole != NULL)
		{
			SetWindowPos(hConsole, HWND_TOPMOST, NULL, NULL, NULL, NULL, SWP_NOMOVE | SWP_NOSIZE | SWP_SHOWWINDOW);
			break;
		}
	}

	return 0;
}

VOID SetConsole()
{
	DWORD mode = 0;
	HANDLE hStdin = 0;
	static FILE* streamconsole;

	AllocConsole();
	AttachConsole(ATTACH_PARENT_PROCESS);
	freopen_s(&streamconsole, "CONIN$", "r+t", stdin);
	freopen_s(&streamconsole, "CONOUT$", "w+t", stdout);
	SetConsoleTitleW(L"CmvsFileHook");

	CreateThread(NULL, NULL, SetConsoleTop, NULL, NULL, NULL);

	hStdin = GetStdHandle(STD_INPUT_HANDLE);
	GetConsoleMode(hStdin, &mode);
	SetConsoleMode(hStdin, mode & ~ENABLE_QUICK_EDIT_MODE);

	//system("chcp 65001");
	//setlocale(LC_ALL, "chs");

	std::locale::global(std::locale(""));
}