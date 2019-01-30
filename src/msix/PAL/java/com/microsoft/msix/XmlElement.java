/*
 * Copyright (C) 2017 Microsoft.  All rights reserved.
 * See LICENSE file in the project root for full license information.
 */
package com.microsoft.msix;

import org.w3c.dom.Document;
import org.w3c.dom.Element;
import org.w3c.dom.NodeList;

import java.util.ArrayList;
import java.util.List;
import javax.xml.xpath.XPath;
import javax.xml.xpath.XPathConstants;
import javax.xml.xpath.XPathFactory;

public class XmlElement {
    private Element m_element;
    private XPath m_xpath;

    public XmlElement(Element element) {
        m_element = element;
        XPathFactory factory = XPathFactory.newInstance();
        m_xpath = factory.newXPath();
    }

    public Element GetElement() {
        return m_element;
    }

    public String GetAttributeValue(String name) {
        return m_element.getAttribute(name);
    }

    public String GetTextContent() {
        return m_element.getTextContent();
    }

    public XmlElement[] GetElementsByTagName(String name) {
        List<XmlElement> elements = new ArrayList<>();
        try {
            NodeList results = m_element.getElementsByTagName(name);;
            for (int i = 0; i < results.getLength(); i++) {
                XmlElement element = new XmlElement((Element) results.item(i));
                elements.add(element);
            }
        } catch(Exception e) {
            e.printStackTrace();
        }
        return elements.toArray(new XmlElement[0]);
    }

    public XmlElement[] GetElements(String query) throws Exception {
        List<XmlElement> elements = new ArrayList<>();
        NodeList results = (NodeList) m_xpath.evaluate(query, m_element, XPathConstants.NODESET);
        for (int i = 0; i < results.getLength(); i++) {
            XmlElement element = new XmlElement((Element) results.item(i));
            elements.add(element);
        }
        return elements.toArray(new XmlElement[0]);
    }
}