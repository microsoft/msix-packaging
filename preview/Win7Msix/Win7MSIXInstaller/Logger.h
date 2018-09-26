#pragma once

extern bool LogCreated;      // keeps track whether the log file is created or not

void SetLogActive();

void SetLogInactive();

void WriteToLog(const char* header, const char* errorMessage);    // logs a message to LOGFILE

void WriteToLogW(const wchar_t* header, const wchar_t* errorMessage);

void HRESULTLog(const wchar_t* title, HRESULT callResult, HRESULT* tracker); // logs an HRESULT