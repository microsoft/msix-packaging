/*
 * Copyright (C) 2017 Microsoft.  All rights reserved.
 * See LICENSE file in the project root for full license information.
 */
package com.microsoft.msix;

import org.xml.sax.ErrorHandler;
import org.xml.sax.SAXParseException;
import org.xml.sax.Locator;

public class JavaXmlException extends SAXParseException {
    // 0: Warning, 1: Error, 2: Fatal.
    int m_errorCode;

    public JavaXmlException(String message, int errorCode)
    {
        super(message, null);
        m_errorCode = errorCode;
    }

    int GetErrorCode()
    {
        return m_errorCode;
    }
}