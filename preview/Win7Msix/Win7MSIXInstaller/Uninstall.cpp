#include <windows.h>
#include <wrl\client.h>
#include <objsafe.h>
#include <objbase.h>
#include "Uninstall.h"
#include <string>
#include <msxml6.h>
#include "RegUtil.h"

#include "MSIXWindows.hpp"
#include "AppxPackaging.hpp"
#pragma comment(lib, "msix.lib")

using namespace std;
using namespace Microsoft::WRL;

#ifdef _WIN64

#else
#endif

#define RETURN_IF_FAILED(a) \
    {   HRESULT __hr = a;   \
        if (FAILED(__hr))   \
        {   return __hr; }  \
    }

class Bstr
{
	BSTR m_bstr;
public:
	operator BSTR() && = delete;
	operator BSTR() & { return m_bstr; }
	Bstr() { m_bstr = nullptr; }
	Bstr(std::wstring text)
	{
		m_bstr = ::SysAllocStringLen(text.c_str(), static_cast<UINT>(text.length()));
	}
	~Bstr() { ::SysFreeString(m_bstr); }

	BSTR* AddressOf()
	{
		::SysFreeString(m_bstr);
		return &m_bstr;
	}

	BSTR& Get() { return m_bstr; }
};

class Variant
{
	VARIANT m_variant;
	bool    m_clear = true;
public:
	operator VARIANT() && = delete;
	operator VARIANT() & { return m_variant; }
	Variant() : m_clear(true) { VariantInit(&m_variant); }
	Variant(bool value) : m_clear(true)
	{
		VariantInit(&m_variant);
		m_variant.vt = VT_BOOL;
		m_variant.boolVal = value ? VARIANT_TRUE : VARIANT_FALSE;
	}
	Variant(IStream* stream) : m_clear(false)
	{
		m_variant.vt = VT_UNKNOWN;
		m_variant.punkVal = stream;
	}
	Variant(Bstr& value) : m_clear(false)
	{
		m_variant.vt = VT_BSTR;
		m_variant.bstrVal = value.Get();
	}

	template <class T>
	Variant(const ComPtr<T>& item) : m_clear(false)
	{
		m_variant.vt = VT_UNKNOWN;
		m_variant.punkVal = item.Get();
	}

	~Variant() { if (m_clear) { VariantClear(&m_variant); } }

	VARIANT* AddressOf()
	{
		VariantClear(&m_variant);
		return &m_variant;
	}

	VARIANT& Get() { return m_variant; }
};

FileEnumerator::FileEnumerator(ComPtr<IXMLDOMDocument> myXmlDocument, ComPtr<IXMLDOMElement> myFileHeader)
{
	m_xmlDocument = myXmlDocument;
	m_fileHeader = myFileHeader;
	m_fileHeader->get_childNodes(&m_fileEnumerator);
	m_fileEnumerator->get_length(&length);
}

std::wstring FileEnumerator::GetCurrent()
{
	if (index >= length)
	{
		return L"";
	}

	IXMLDOMNode* child;
	IXMLDOMNode* text;
	m_fileEnumerator->get_item(index, &child);
	BSTR filePath;
	child->get_firstChild(&text);
	text->get_text(&filePath);
	return filePath;
}

bool FileEnumerator::HasNext()
{
	return index < length - 1;
}

bool FileEnumerator::MoveNext()
{
	if (index < length - 1)
	{
		index++;
		return true;
	}
	else
	{
		return false;
	}
}

DirectoryEnumerator::DirectoryEnumerator(ComPtr<IXMLDOMDocument> myXmlDocument, ComPtr<IXMLDOMElement> myDirHeader)
{
	m_xmlDocument = myXmlDocument;
	m_dirHeader = myDirHeader;
	m_dirHeader->get_childNodes(&m_dirEnumerator);
	m_dirEnumerator->get_length(&length);
	index = length - 1;
}

std::wstring DirectoryEnumerator::GetCurrent()
{
	if (index < 0)
	{
		return L"";
	}

	IXMLDOMNode* child;
	IXMLDOMNode* text;
	m_dirEnumerator->get_item(index, &child);
	BSTR dirPath;
	child->get_firstChild(&text);
	text->get_text(&dirPath);
	return dirPath;
}

bool DirectoryEnumerator::HasNext()
{
	return index > 0;
}

bool DirectoryEnumerator::MoveNext()
{
	if (index > 0)
	{
		index--;
		return true;
	}
	else
	{
		return false;
	}
}

RegistryEnumerator::RegistryEnumerator(ComPtr<IXMLDOMDocument> myXmlDocument, ComPtr<IXMLDOMElement> myRegHeader)
{
	m_xmlDocument = myXmlDocument;
	m_regHeader = myRegHeader;
	m_regHeader->get_childNodes(&m_regEnumerator);
	m_regEnumerator->get_length(&length);
	index = length - 1;
}

std::wstring RegistryEnumerator::GetCurrent(bool* isx64)
{
	if (index < 0)
	{
		return L"";
	}

	ComPtr<IXMLDOMNode> child;
	ComPtr<IXMLDOMElement> childElem;
	ComPtr<IXMLDOMNode> text;
	m_regEnumerator->get_item(index, &child);

	Bstr versionHeader(L"Is64");
	Variant versionVar;
	Bstr regKey;
	child.As<IXMLDOMElement>(&childElem);
	HRESULT hr = childElem->getAttribute(versionHeader, versionVar.AddressOf());
	if (SUCCEEDED(hr))
	{
		Bstr temp1(versionVar.Get().bstrVal);
		std::wstring temp2(temp1.Get(), SysStringLen(temp1.Get()));
		bool temp = temp2 != L"0";
		
		*isx64 = temp;
		child->get_firstChild(&text);
		text->get_text(regKey.AddressOf());
	}

	return regKey.Get();
}

bool RegistryEnumerator::HasNext()
{
	return index > 0;
}

bool RegistryEnumerator::MoveNext()
{
	if (index > 0)
	{
		index--;
		return true;
	}
	else
	{
		return false;
	}
}

// Here's how to initialize a blank file:
// TrackerXML myXML();
// myXML.SetXMLName(L"C:\\My\\Directory", L"fileName");

// Here's how to load XML:
// TrackerXML myXML(L"C:\\Path\\To\\file.xml");

// Here's how to do your FileEnumerator:
// FileEnumerator fileEnum = myXML.GetFileEnumerator();

TrackerXML::TrackerXML()
{
	HRESULT hr = Initialize();
	if (hr != ERROR_SUCCESS)
	{
		return;
	}
}

TrackerXML::TrackerXML(std::wstring loadSource)
{
	LoadXML(loadSource);
}

HRESULT TrackerXML::Initialize()
{
#ifdef _WIN64
	is64 = true;
#else
	is64 = false;
#endif

	Variant vTrue(true);
	Bstr rootNode(L"<Components></Components>");
	VARIANT_BOOL success(VARIANT_FALSE);

	RETURN_IF_FAILED(CoInitializeEx(NULL, COINIT_MULTITHREADED));

	RETURN_IF_FAILED(CoCreateInstance(__uuidof(DOMDocument60), nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&m_xmlDocument)));

	RETURN_IF_FAILED(m_xmlDocument->loadXML(rootNode, &success));

	ComPtr<IXMLDOMElement> dom;
	m_xmlDocument->get_documentElement(&dom);

	Bstr target(L"xml");
	Bstr data(L" version='1.0' encoding='UTF-8'");
	ComPtr<IXMLDOMProcessingInstruction> xmlProcessingNode;
	m_xmlDocument->createProcessingInstruction(target, data, &xmlProcessingNode);

	Variant vtObject;
	vtObject.Get().vt = VT_DISPATCH;
	vtObject.Get().pdispVal = dom.Get();
	vtObject.Get().pdispVal->AddRef();

	ComPtr<IXMLDOMNode> node;
	m_xmlDocument->insertBefore(xmlProcessingNode.Get(), vtObject, &node);

	Bstr disNStr(L"DisplayName");
	IXMLDOMNode* out;
	m_xmlDocument->createElement(disNStr, &m_displayName);
	dom->appendChild(m_displayName.Get(), &out);

	Bstr dirStr(L"Directories");
	m_xmlDocument->createElement(dirStr, &m_dirHeader);
	dom->appendChild(m_dirHeader.Get(), &out);

	Bstr fileStr(L"Files");
	m_xmlDocument->createElement(fileStr, &m_fileHeader);
	dom->appendChild(m_fileHeader.Get(), &out);

	Bstr packStr(L"PackageFullName");
	m_xmlDocument->createElement(packStr, &m_packageName);
	dom->appendChild(m_packageName.Get(), &out);

	Bstr regStr(L"Registry");
	m_xmlDocument->createElement(regStr, &m_regHeader);
	dom->appendChild(m_regHeader.Get(), &out);

	return ERROR_SUCCESS;
}

HRESULT TrackerXML::LoadXML(std::wstring loadSource)
{
#ifdef _WIN64
	is64 = true;
#else
	is64 = false;
#endif

	//Load the COM
	Variant vTrue(true);
	VARIANT_BOOL success(VARIANT_FALSE);

	RETURN_IF_FAILED(CoInitializeEx(NULL, COINIT_MULTITHREADED));

	RETURN_IF_FAILED(CoCreateInstance(__uuidof(DOMDocument60), nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&m_xmlDocument)));

	//Load the XML Document
	Bstr path(loadSource);
	Variant loadPath(path);
	RETURN_IF_FAILED(m_xmlDocument->load(loadPath, &success));

	//Assign the other class pointers
	Bstr disNHeaderName(L"DisplayName");
	ComPtr<IXMLDOMNodeList> outListDisN;
	RETURN_IF_FAILED(m_xmlDocument->getElementsByTagName(disNHeaderName, &outListDisN));
	ComPtr<IXMLDOMNode> outDisN;
	RETURN_IF_FAILED(outListDisN->get_item(0, &outDisN));
	RETURN_IF_FAILED(outDisN.As<IXMLDOMElement>(&m_displayName));

	Bstr dirHeaderName(L"Directories");
	ComPtr<IXMLDOMNodeList> outListDir;
	RETURN_IF_FAILED(m_xmlDocument->getElementsByTagName(dirHeaderName, &outListDir));
	ComPtr<IXMLDOMNode> outDir;
	RETURN_IF_FAILED(outListDir->get_item(0, &outDir));
	RETURN_IF_FAILED(outDir.As<IXMLDOMElement>(&m_dirHeader));

	Bstr fileHeaderName(L"Files");
	ComPtr<IXMLDOMNodeList> outListFile;
	RETURN_IF_FAILED(m_xmlDocument->getElementsByTagName(fileHeaderName, &outListFile));
	ComPtr<IXMLDOMNode> outFile;
	RETURN_IF_FAILED(outListFile->get_item(0, &outFile));
	RETURN_IF_FAILED(outFile.As<IXMLDOMElement>(&m_fileHeader));

	Bstr packHeaderName(L"PackageFullName");
	ComPtr<IXMLDOMNodeList> outListPack;
	RETURN_IF_FAILED(m_xmlDocument->getElementsByTagName(packHeaderName, &outListPack));
	ComPtr<IXMLDOMNode> outPack;
	RETURN_IF_FAILED(outListPack->get_item(0, &outPack));
	RETURN_IF_FAILED(outPack.As<IXMLDOMElement>(&m_packageName));

	Bstr regHeaderName(L"Registry");
	ComPtr<IXMLDOMNodeList> outListReg;
	RETURN_IF_FAILED(m_xmlDocument->getElementsByTagName(regHeaderName, &outListReg));
	ComPtr<IXMLDOMNode> outReg;
	RETURN_IF_FAILED(outListReg->get_item(0, &outReg));
	RETURN_IF_FAILED(outReg.As<IXMLDOMElement>(&m_regHeader));

	return S_OK;
}

void TrackerXML::SetXMLName(std::wstring directory, std::wstring name)
{
	fileName = directory + L"\\" + name + L".xml";
}

long TrackerXML::GetFilesLength()
{
	ComPtr<IXMLDOMNodeList> fileList;
	m_fileHeader->get_childNodes(&fileList);
	long length;
	fileList->get_length(&length);
	return length;
}

void TrackerXML::SaveFile()
{
	Bstr locStr(fileName);
	Variant loc(locStr);
	m_xmlDocument->save(loc);
}

std::wstring TrackerXML::GetDisplayName()
{
	ComPtr<IXMLDOMNodeList> children;
	m_displayName->get_childNodes(&children);
	long childrenSize;
	children->get_length(&childrenSize);

	if (childrenSize > 0)
	{
		ComPtr<IXMLDOMNode> text;
		m_displayName->get_firstChild(&text);
		BSTR displayName;
		text->get_text(&displayName);
		return displayName;
	}

	return L"";
}

void TrackerXML::SetDisplayName(std::wstring displayName)
{
	ComPtr<IXMLDOMNodeList> children;
	m_displayName->get_childNodes(&children);
	long childrenSize;
	children->get_length(&childrenSize);

	Bstr displayStr(displayName);
	ComPtr<IXMLDOMText> disText;
	ComPtr<IXMLDOMNode> out;
	m_xmlDocument->createTextNode(displayStr, &disText);

	if (childrenSize > 0)
	{
		ComPtr<IXMLDOMNode> oldName;
		m_displayName->get_firstChild(&oldName);
		m_displayName->replaceChild(disText.Get(), oldName.Get(), &oldName);
	}
	else
	{
		m_displayName->appendChild(disText.Get(), &out);
	}
}

void TrackerXML::AddDirectoryToRecord(std::wstring dirName)
{
	ComPtr<IXMLDOMElement> dir;
	Bstr dirStr(L"Directory");
	m_xmlDocument->createElement(dirStr, &dir);

	Bstr folderStr(dirName);
	IXMLDOMText* dirText;
	IXMLDOMNode* out;
	m_xmlDocument->createTextNode(folderStr, &dirText);
	dir->appendChild(dirText, &out);
	m_dirHeader->appendChild(dir.Get(), &out);

	//Manually release non-ComPtrs
	dirText->Release();
	out->Release();

	SaveFile();
}

void TrackerXML::AddFileToRecord(std::wstring filePath)
{
	ComPtr<IXMLDOMElement> file;
	Bstr fileStr(L"File");
	m_xmlDocument->createElement(fileStr, &file);

	Bstr pathStr(filePath);
	IXMLDOMText* fileText;
	IXMLDOMNode* out;
	m_xmlDocument->createTextNode(pathStr, &fileText);
	file->appendChild(fileText, &out);
	m_fileHeader->appendChild(file.Get(), &out);

	//Manually release non-ComPtrs
	fileText->Release();
	out->Release();

	SaveFile();
}

std::wstring TrackerXML::GetPackageName()
{
	ComPtr<IXMLDOMNodeList> children;
	m_packageName->get_childNodes(&children);
	long childrenSize;
	children->get_length(&childrenSize);

	if (childrenSize > 0)
	{
		ComPtr<IXMLDOMNode> text;
		m_packageName->get_firstChild(&text);
		BSTR packageName;
		text->get_text(&packageName);
		return packageName;
	}

	return L"";
}

void TrackerXML::SetPackageName(std::wstring packageName)
{
	ComPtr<IXMLDOMNodeList> children;
	m_packageName->get_childNodes(&children);
	long childrenSize;
	children->get_length(&childrenSize);

	Bstr packageStr(packageName);
	ComPtr<IXMLDOMText> packText;
	ComPtr<IXMLDOMNode> out;
	m_xmlDocument->createTextNode(packageStr, &packText);

	if (childrenSize > 0)
	{
		ComPtr<IXMLDOMNode> oldName;
		m_packageName->get_firstChild(&oldName);
		m_packageName->replaceChild(packText.Get(), oldName.Get(), &oldName);
	}
	else
	{
		m_packageName->appendChild(packText.Get(), &out);
	}
}

void TrackerXML::AddRegKeyToRecord(HKEY hKey, LPCWSTR lpSubKey)
{
	std::wstring fullKey = GetHiveName(hKey) + L"\\" + lpSubKey;

	ComPtr<IXMLDOMElement> key;
	Bstr keyStr(L"Key");
	m_xmlDocument->createElement(keyStr, &key);

	Bstr versionHeader(L"Is64");
	Variant versionBool(is64);
	key->setAttribute(versionHeader, versionBool);

	Bstr pathStr(fullKey);
	IXMLDOMText* keyText;
	IXMLDOMNode* out;
	m_xmlDocument->createTextNode(pathStr, &keyText);
	key->appendChild(keyText, &out);
	m_regHeader->appendChild(key.Get(), &out);

	//Manually release non-ComPtrs
	keyText->Release();
	out->Release();

	SaveFile();
}

FileEnumerator TrackerXML::GetFileEnumerator()
{
	return FileEnumerator(m_xmlDocument, m_fileHeader);
}

DirectoryEnumerator TrackerXML::GetDirectoryEnumerator()
{
	return DirectoryEnumerator(m_xmlDocument, m_dirHeader);
}

RegistryEnumerator TrackerXML::GetRegistryEnumerator()
{
	return RegistryEnumerator(m_xmlDocument, m_regHeader);
}
