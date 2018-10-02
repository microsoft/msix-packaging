//
//  Copyright (C) 2017 Microsoft.  All rights reserved.
//  See LICENSE file in the project root for full license information.
// 
#pragma once
#include "NSXmlParserWrapper.h"
#include <string>
#include <list>
#include <stack>
#include <map>

namespace MSIX {

class XmlAttribute
{
public:
    std::string Name;
    std::string Value;
};

class XmlNode
{
public:
    std::map<std::string, std::string> Attributes;
    std::list<std::unique_ptr<XmlNode>> Children;
    std::string Text;
    std::string NodeName;
    std::string QualifiedNodeName;
    
    std::list<XmlNode*> FindElements(std::string xpath);
private:
    void FindElementsRecursive(std::string xpath, std::list<XmlNode*>& list);
};

class XmlDocumentReader
{
public:
    void Init();
   
    bool Parse(uint8_t* data, size_t size);
    
    void ProcessNodeBegin(std::unique_ptr<XmlNode> node);
    void ProcessNodeEnd(std::string nodeName);
    void ProcessCharacters(std::string string);
    
    XmlNode* GetRoot(){return m_root.get();};
      
private:
    NSXmlParserWrapper* m_wrapper;
    std::unique_ptr<XmlNode> m_root;
    std::stack<XmlNode*> m_currentNodeStack;
};
}
