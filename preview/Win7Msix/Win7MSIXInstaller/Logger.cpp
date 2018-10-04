#include <windows.h>
#include <string>
#include <memory>
#include <stdlib.h>
#include <stdio.h>
#include <chrono>
#include "Logger.h"
#include <time.h>
#include <direct.h>
#include <comdef.h>
#include <locale>
#include <codecvt>

using namespace std;

bool LogCreated = false;
bool LogActive = false;
std::unique_ptr<string> FileName;

//Determines if the log for that day has been created
bool FileExists(const char* filename)
{
	FILE *fp;
	fopen_s(&fp, filename, "r");
	if (fp != NULL) fclose(fp);
	return (fp != NULL);
}

void GetFile(FILE** file)
{
	if (!LogCreated) {
		//Get the current date in a readable format
		time_t rawtime;
		struct tm timeinfo;
		time(&rawtime);
		localtime_s(&timeinfo, &rawtime);

		//Makes log file name "logdate[mmddyyyy].log"
		string logFileName = "logdate" + to_string(timeinfo.tm_mon + 1) +
			to_string(timeinfo.tm_mday) + to_string(1900 + timeinfo.tm_year) +
			".log";
		FileName = std::make_unique<string>(logFileName);

		if (!FileExists(logFileName.c_str()))
		{
			fopen_s(file, logFileName.c_str(), "w");
			LogCreated = true;
		}
		else
		{
			fopen_s(file, logFileName.c_str(), "a");
			LogCreated = true;
		}
	}
	else
		fopen_s(file, (*FileName).c_str(), "a");
}

void Log(const char* message)
{
	FILE *file;

	GetFile(&file);

	if (file == NULL) {
		if (LogCreated)
			LogCreated = false;
		return;
	}
	else
	{
		fputs(message, file);
		fputs("\n", file);
		fclose(file);
	}

	if (file)
		fclose(file);
}

void LogW(const wchar_t* message)
{
	FILE *file;

	GetFile(&file);

	if (file == NULL) {
		if (LogCreated)
			LogCreated = false;
		return;
	}
	else
	{
		fputws(message, file);
		fputs("\n", file);
		fclose(file);
	}

	if (file)
		fclose(file);
}

void SetLogActive()
{
	LogActive = true;
}

void SetLogInactive()
{
	LogActive = false;
}

void WriteToLog(const char* header, const char* errorMessage)
{
	if (!LogActive)
	{
		return;
	}
	string colon = ": ";
	Log((header + colon + errorMessage).c_str());
}

void WriteToLogW(const wchar_t* header, const wchar_t* errorMessage)
{
	if (!LogActive)
	{
		return;
	}
	wstring colon = L": ";
	LogW((header + colon + errorMessage).c_str());
}

void HRESULTLog(const wchar_t* title, HRESULT callResult, HRESULT* tracker)
{
	_com_error err(callResult);
	WriteToLogW(title, err.ErrorMessage());
	tracker = &callResult;
}