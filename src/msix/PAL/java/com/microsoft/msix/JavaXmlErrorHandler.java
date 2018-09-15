/*
 * Copyright (C) 2017 Microsoft.  All rights reserved.
 * See LICENSE file in the project root for full license information.
 */
package com.microsoft.msix;

import com.microsoft.msix.JavaXmlException;

import org.xml.sax.ErrorHandler;
import org.xml.sax.SAXParseException;

public class JavaXmlErrorHandler implements ErrorHandler {

    @Override
    public void warning(SAXParseException exc) throws JavaXmlException{
        throw new JavaXmlException(exc.toString(), 0 /*warning*/);
    }

    @Override
    public void error(SAXParseException exc) throws JavaXmlException{
        throw new JavaXmlException(exc.toString(), 1 /*error */);
    }

    @Override
    public void fatalError(SAXParseException exc) throws SAXParseException {
        // for fatalError we simply rethrow the SAX exception.
        throw exc;
    }
}