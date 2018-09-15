/*
 * Copyright (C) 2017 Microsoft.  All rights reserved.
 * See LICENSE file in the project root for full license information.
 */
package com.microsoft.msix;

import org.w3c.dom.Document;
import org.w3c.dom.Element;
import org.w3c.dom.NodeList;

import java.io.ByteArrayInputStream;
import java.io.InputStream;
import java.util.ArrayList;
import java.util.List;

import javax.xml.parsers.DocumentBuilder;
import javax.xml.parsers.DocumentBuilderFactory;
import javax.xml.xpath.XPath;
import javax.xml.xpath.XPathConstants;
import javax.xml.xpath.XPathFactory;

public class XmlDom {
    private Document m_document;
    private XPath m_xpath;

    public XmlDom() {
        m_document = null;
        XPathFactory factory = XPathFactory.newInstance();
        m_xpath = factory.newXPath();
    }

    public void InitializeDocument(byte[] stream) throws Exception {
        InputStream is = new ByteArrayInputStream(stream);
        DocumentBuilder builder = DocumentBuilderFactory.newInstance().newDocumentBuilder();
        builder.setErrorHandler(new JavaXmlErrorHandler());
        m_document = builder.parse(is);

    }

    public XmlElement GetDocument() {
        XmlElement dom = new XmlElement(m_document.getDocumentElement());
        return dom;
    }

    public XmlElement[] GetElements(XmlElement root, String query) throws Exception{
        List<XmlElement> elements = new ArrayList<>();
        NodeList results = (NodeList) m_xpath.evaluate(query, root.GetElement(), XPathConstants.NODESET);
        for (int i = 0; i < results.getLength(); i++) {
            XmlElement element = new XmlElement((Element) results.item(i));
            elements.add(element);
        }
        return elements.toArray(new XmlElement[0]);
    }
}