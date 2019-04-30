//
//  Copyright (C) 2019 Microsoft.  All rights reserved.
//  See LICENSE file in the project root for full license information.
// 
#pragma once

#include "ComHelper.hpp"
#include "StringStream.hpp"

#include <stack>
#include <string>

namespace MSIX {

    // common attribute names
    static const char* xmlnsAttribute = "xmlns";

    // This is a super light xml writer that doesn't use any xml libraries and 
    // just writes to a stream the basics of an xml file.
    class XmlWriter final
    {
    public:
        typedef enum
        {
            OpenElement = 1,
            ClosedElement = 2,
            Finish = 3
        }
        State;

        XmlWriter(const std::string& root) 
        {
            m_stream = ComPtr<IStream>::Make<StringStream>();
            StartWrite(root);
        }

        XmlWriter(const std::string& root, IStream* targetStream) : m_stream(ComPtr<IStream>(targetStream))
        {
            StartWrite(root);
        }

        void StartElement(const std::string& name);
        void CloseElement();
        void AddAttribute(const std::string& name, const std::string& value);
        State GetState() { return m_state; }
        ComPtr<IStream> GetStream();

    protected:
        void StartWrite(const std::string& root);
        void Write(const std::string& toWrite);
        State m_state;
        ComPtr<IStream> m_stream;
        std::stack<std::string> m_elements;
    };
}
