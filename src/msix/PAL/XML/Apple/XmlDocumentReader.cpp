//
//  Copyright (C) 2017 Microsoft.  All rights reserved.
//  See LICENSE file in the project root for full license information.
// 
#include "XmlDocumentReader.hpp"
#include "Exceptions.hpp"

namespace MSIX {

void XmlDocumentReader::Init()
{
    m_wrapper = std::make_unique<NSXmlParserWrapper>();
}

bool XmlDocumentReader::Parse(uint8_t* data, size_t size)
{
    return m_wrapper->Parse(data, size, this);
}

void XmlDocumentReader::ProcessNodeBegin(std::unique_ptr<XmlNode> node)
{
    if (m_currentNodeStack.empty())
    {
        m_currentNodeStack.push(node.get());
        m_root.swap(node);
    }
    else
    {
        auto currentNode = m_currentNodeStack.top();
        m_currentNodeStack.push(node.get());
        currentNode->Children.emplace_back(std::move(node));
    }
}

void XmlDocumentReader::ProcessNodeEnd(std::string nodeName)
{
    if (!m_currentNodeStack.empty())
    {
        auto currentNode = m_currentNodeStack.top();
        ThrowErrorIf(Error::XmlFatal, nodeName.compare(currentNode->NodeName) != 0, "Node end does not match current node opened.");
        m_currentNodeStack.pop();
    }
}

void XmlDocumentReader::ProcessCharacters(std::string string)
{
    auto currentNode = m_currentNodeStack.top();
    currentNode->Text.append(string);
}

void XmlNode::FindElementsRecursive(std::string xpath, std::list<XmlNode*>& list)
{
    size_t nextPathSeparatorIndex = xpath.find_first_of("//");
    std::string currentXpathSegment  = xpath.substr(0, nextPathSeparatorIndex);
    
    if (NodeName.compare(currentXpathSegment) == 0)
    {
        if (nextPathSeparatorIndex == std::string::npos)
        {
            list.emplace_back(this);
        }
        for(auto it = Children.begin(); it != Children.end(); ++it)
        {
            (*it)->FindElementsRecursive(xpath.substr(nextPathSeparatorIndex + 1), list);
        }
    }
}

std::list<XmlNode*> XmlNode::FindElements(std::string xpath)
{
    std::list<XmlNode*> list;
    
    if (xpath.size() >= 2 && xpath[0] == '.' && xpath[1] == '/')
    {
        std::string newXPath = xpath.substr(2);
        for(auto it = Children.begin(); it != Children.end(); ++it)
        {
            auto child = (*it).get();
            child->FindElementsRecursive(newXPath, list);
        }
    }
    else  if (xpath.size() > 1 && xpath[0] == '/')
    {
        std::string newXPath = xpath.substr(1);
        FindElementsRecursive(newXPath, list);
    }
    return list;
}
}
