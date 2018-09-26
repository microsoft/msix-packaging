#pragma once
#include <windows.h>
#include <wrl\client.h>
#include <string>
#include <msxml6.h>

class FileEnumerator {
public:
	FileEnumerator(Microsoft::WRL::ComPtr<IXMLDOMDocument> myXmlDocument, Microsoft::WRL::ComPtr<IXMLDOMElement> myFileHeader);
	std::wstring GetCurrent();
	bool HasNext();
	bool MoveNext();
private:
	long index = 0;
	long length;
	Microsoft::WRL::ComPtr<IXMLDOMNodeList> m_fileEnumerator;
	Microsoft::WRL::ComPtr<IXMLDOMDocument> m_xmlDocument;
	Microsoft::WRL::ComPtr<IXMLDOMElement> m_fileHeader;
};

class DirectoryEnumerator {
public:
	DirectoryEnumerator(Microsoft::WRL::ComPtr<IXMLDOMDocument> myXmlDocument, Microsoft::WRL::ComPtr<IXMLDOMElement> myFileHeader);
	std::wstring GetCurrent();
	bool HasNext();
	bool MoveNext();
private:
	long index;
	long length;
	Microsoft::WRL::ComPtr<IXMLDOMNodeList> m_dirEnumerator;
	Microsoft::WRL::ComPtr<IXMLDOMDocument> m_xmlDocument;
	Microsoft::WRL::ComPtr<IXMLDOMElement> m_dirHeader;
};

class RegistryEnumerator {
public:
	RegistryEnumerator(Microsoft::WRL::ComPtr<IXMLDOMDocument> myXmlDocument, Microsoft::WRL::ComPtr<IXMLDOMElement> myRegHeader);
	std::wstring GetCurrent(bool* isx64);
	bool HasNext();
	bool MoveNext();
private:
	long index;
	long length;
	Microsoft::WRL::ComPtr<IXMLDOMNodeList> m_regEnumerator;
	Microsoft::WRL::ComPtr<IXMLDOMDocument> m_xmlDocument;
	Microsoft::WRL::ComPtr<IXMLDOMElement> m_regHeader;
};

class TrackerXML {
public:
	TrackerXML();
	TrackerXML(std::wstring loadSource);
	HRESULT Initialize();
	HRESULT LoadXML(std::wstring loadSource);
	void SetXMLName(std::wstring directory, std::wstring name);
	void AddFileToRecord(std::wstring filePath);
	void AddDirectoryToRecord(std::wstring dirName);
	std::wstring GetPackageName();
	void SetPackageName(std::wstring packageName);
	void AddRegKeyToRecord(HKEY hKey, LPCWSTR lpSubKey);
	std::wstring GetDisplayName();
	void SetDisplayName(std::wstring displayName);
	long GetFilesLength();
	FileEnumerator GetFileEnumerator();
	DirectoryEnumerator GetDirectoryEnumerator();
	RegistryEnumerator GetRegistryEnumerator();
private:
	bool is64 = false;
	bool fileCreated = false;
	std::wstring fileName = L"text.xml";
	std::wstring fileHeaderName = L"Files";
	std::wstring dirHeaderName = L"Directories";
	std::wstring disNHeaderName = L"DisplayName";
	std::wstring regHeaderName = L"Registry";
	void SaveFile();
	Microsoft::WRL::ComPtr<IXMLDOMDocument> m_xmlDocument;
	Microsoft::WRL::ComPtr<IXMLDOMElement> m_fileHeader;
	Microsoft::WRL::ComPtr<IXMLDOMElement> m_dirHeader;
	Microsoft::WRL::ComPtr<IXMLDOMElement> m_displayName;
	Microsoft::WRL::ComPtr<IXMLDOMElement> m_regHeader;
	Microsoft::WRL::ComPtr<IXMLDOMElement> m_packageName;
};
