#pragma once
#ifndef _CONSOLE_H_
#define _CONSOLE_H_

#include <io.h>
#include <fcntl.h> 
#include <iostream>

namespace Console
{
	void CreateConsole(char* consoleTitle)
	{
		int hConHandle = 0;
		HANDLE lStdHandle = 0;
		FILE *fp = 0;

		// Allocate a console
		AllocConsole();

		// redirect unbuffered STDOUT to the console
		lStdHandle = GetStdHandle(STD_OUTPUT_HANDLE);
		hConHandle = _open_osfhandle(PtrToUlong(lStdHandle), _O_TEXT);
		fp = _fdopen(hConHandle, "w");
		*stdout = *fp;
		setvbuf(stdout, NULL, _IONBF, 0);

		// redirect unbuffered STDIN to the console
		lStdHandle = GetStdHandle(STD_INPUT_HANDLE);
		hConHandle = _open_osfhandle(PtrToUlong(lStdHandle), _O_TEXT);
		fp = _fdopen(hConHandle, "r");
		*stdin = *fp;
		setvbuf(stdin, NULL, _IONBF, 0);

		// redirect unbuffered STDERR to the console
		lStdHandle = GetStdHandle(STD_ERROR_HANDLE);
		hConHandle = _open_osfhandle(PtrToUlong(lStdHandle), _O_TEXT);
		fp = _fdopen(hConHandle, "w");
		*stderr = *fp;
		setvbuf(stderr, NULL, _IONBF, 0);

		// Title
		SetConsoleTitleA(consoleTitle);
	}

	void print(const char* text, ...)
	{
		static HANDLE lStdHandle = GetStdHandle(STD_OUTPUT_HANDLE);

		char Buffer[1024 * 4] = { 0 };
		va_list va_alist;
		va_start(va_alist, text);
		vsnprintf(Buffer, sizeof(Buffer), text, va_alist);
		va_end(va_alist);

		LPDWORD i = 0;
		WriteConsole(lStdHandle, Buffer, strlen(Buffer), i, NULL);

		i = 0;
		WriteConsole(lStdHandle, "\n", 1, i, NULL);
	}
}


#endif