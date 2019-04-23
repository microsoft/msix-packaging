//
//  Copyright (C) 2019 Microsoft.  All rights reserved.
//  See LICENSE file in the project root for full license information.
//
#include "XmlWriter.hpp"
#include "StringStream.hpp"
#include "ComHelper.hpp"
#include "Exceptions.hpp"
#include "MsixErrors.hpp"

#include <stack>
#include <string>

namespace MSIX {

        const static char* xmlStart = "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"true\"?><";

        // Adds xml header declaration plus the name of the root element
        XmlWriter::XmlWriter(const std::string& root)
        {
            m_elements.emplace(root);
            m_stream = ComPtr<IStream>::Make<StringStream>();
            std::string start = xmlStart + root;
            ULONG copy;
            ThrowHrIfFailed(m_stream->Write(static_cast<const void*>(start.data()), static_cast<ULONG>(start.size()), &copy));
            m_state = State::OpenElement;
        }

        void XmlWriter::StartElement(const std::string& name)
        {
            ThrowErrorIf(Error::XmlError, m_state == State::Finish, "Invalid call, xml already closed");
            m_elements.emplace(name);
            std::string element;
            // If the state is open, then we are adding a child element to the previous one. We need to close that element's
            // tag and add the new one. If the state is closed, there is no need to close the tag as it had already been closed.
            if (m_state == State::OpenElement)
            {
                // close parent element
                element = "><" + name;
            }
            else // State::ClosedElement
            {
                element = "<" + name;
            }
            ULONG copy;
            ThrowHrIfFailed(m_stream->Write(static_cast<const void*>(element.data()), static_cast<ULONG>(element.size()), &copy));
            m_state = State::OpenElement;
        }

        void XmlWriter::CloseElement()
        {
            ThrowErrorIf(Error::XmlError, m_state == State::Finish, "Invalid call, xml already closed");
            std::string close;
            // If the state is open and we are closing an element, it means that it doesn't have any child, so we can
            // just close it with "/>". If we are closing an element and a closing just happened, it means that we are 
            // closing an element that has child elements, so it must be closed with </element>
            if (m_state == State::OpenElement)
            {
                close = "/>";
            }
            else // State::ClosedElement
            {
                close = "</" + m_elements.top() + ">"; // </name>
            }
            ULONG copy = 0;
            ThrowHrIfFailed(m_stream->Write(static_cast<const void*>(close.data()), static_cast<ULONG>(close.size()), &copy));
            m_state = State::ClosedElement;
            m_elements.pop();
            if (m_elements.size() == 0)
            {
                m_state = State::Finish;
            }
        }

        void XmlWriter::AddAttribute(const std::string& name, const std::string& value)
        {
            ThrowErrorIf(Error::XmlError, (m_state == State::Finish) || (m_state == State::ClosedElement), "Invalid call to AddAttrbute");
            // Name="Value". Always add space at the beginning.
            std::string attribute = " " + name + "=\"" + value + "\"";
            ULONG copy;
            ThrowHrIfFailed(m_stream->Write(static_cast<const void*>(attribute.data()), static_cast<ULONG>(attribute.size()), &copy));
        }
}
