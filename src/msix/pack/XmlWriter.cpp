//
//  Copyright (C) 2019 Microsoft.  All rights reserved.
//  See LICENSE file in the project root for full license information.
//
#include "XmlWriter.hpp"
#include "MemoryStream.hpp"
#include "ComHelper.hpp"
#include "Exceptions.hpp"
#include "MsixErrors.hpp"
#include "StreamHelper.hpp"

#include <stack>
#include <string>

namespace MSIX {

    void XmlWriter::Initialize(const std::string& source, const std::string& root)
    {
        // Verify that the string actually ends with the proper end element
        std::string endElementString;
        endElementString += "</";
        endElementString += root;
        endElementString += ">";

        ThrowErrorIf(Error::InvalidParameter, source.length() < endElementString.length(), "not enough bytes in string");

        std::string endElementCandidate = source.substr(source.length() - endElementString.length());
        ThrowErrorIf(Error::InvalidParameter, endElementCandidate != endElementString, "stream did not end with end element");

        // Write out everything but the end element
        m_stream = ComPtr<IStream>::Make<MemoryStream>();

        ULONG toWrite = static_cast<ULONG>(source.length() - endElementString.length());
        ULONG written = 0;
        ThrowHrIfFailed(m_stream->Write(static_cast<const void*>(source.data()), toWrite, &written));
        ThrowErrorIf(Error::FileWrite, (toWrite != written), "write failed");

        // Set us up to close things out later
        m_elements.emplace(root);
        m_state = State::ClosedElement;
    }

    const static char* xmlStart = "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"";
    const static char* xmlStartEnd = "\"?><";

    // Adds xml header declaration plus the name of the root element
    void XmlWriter::StartWrite(const std::string& root, bool standalone)
    {
        m_elements.emplace(root);
        Write(xmlStart);
        if (standalone)
        {
            Write("yes");
        }
        else
        {
            Write("no");
        }
        Write(xmlStartEnd);
        Write(root);
        m_state = State::OpenElement;
    }

    void XmlWriter::StartElement(const std::string& name)
    {
        ThrowErrorIf(Error::XmlError, m_state == State::Finish, "Invalid call, xml already finished");
        m_elements.emplace(name);
        // If the state is open, then we are adding a child element to the previous one. We need to close that element's
        // tag and add the new one. If the state is closed, there is no need to close the tag as it had already been closed.
        if (m_state == State::OpenElement)
        {
            Write(">"); // close parent element
        }
        Write("<");
        Write(name);
        m_state = State::OpenElement;
    }

    void XmlWriter::CloseElement()
    {
        ThrowErrorIf(Error::XmlError, m_state == State::Finish, "Invalid call, xml already finished");
        // If the state is open and we are closing an element, it means that it doesn't have any child, so we can
        // just close it with "/>". If we are closing an element and a closing just happened, it means that we are 
        // closing an element that has child elements, so it must be closed with </element>
        if (m_state == State::OpenElement)
        {
            Write("/>");
        }
        else // State::ClosedElement
        {
            // </name>
            Write("</");
            Write(m_elements.top());
            Write(">");
        }
        m_state = State::ClosedElement;
        m_elements.pop();
        if (m_elements.size() == 0)
        {
            m_state = State::Finish;
        }
    }

    void XmlWriter::AddAttribute(const std::string& name, const std::string& value)
    {
        ThrowErrorIf(Error::XmlError, (m_state == State::Finish) || (m_state == State::ClosedElement), "Invalid call to AddAttribute");
        Write(" "); // always write a space. We just wrote either an element or an attribute
        Write(name); // name="value"
        Write("=\"");
        WriteTextValue(value);
        Write("\"");
    }

    ComPtr<IStream> XmlWriter::GetStream()
    {
        ThrowErrorIf(Error::XmlError, m_state != State::Finish, "Invalid call, the stream can only be accessed when the writer is done");
        return m_stream;
    }

// Following msxml6 rule for text values
//  all ampersands (&) are replaced by &amp;
//  all open angle brackets (<) are replaced by &lt;
//  all closing angle brackets (>) are replaced by &gt;
//  and all #xD characters are replaced by &#xD; 
    void XmlWriter::WriteTextValue(const std::string& value)
    {
        for (size_t i = 0; i < value.size(); i++)
        {
            if (value[i] == '&')
            {
                Write("&amp;");
            }
            else if (value[i] == '<')
            {
                Write("&lt;");
            }
            else if (value[i] == '>')
            {
                Write("&gt;");
            }
            else if (value[i] == 0xd)
            {
                Write("&#xD;");
            }
            else
            {
                Write(value[i]);
            }
        }
    }

    void XmlWriter::Write(const std::string& toWrite)
    {
        Helper::WriteStringToStream(m_stream, toWrite);
    }

    void XmlWriter::Write(const char toWrite)
    {
        Helper::WriteStringToStream(m_stream, std::string(1, toWrite));
    }
}
