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

import javax.xml.XMLConstants;
import javax.xml.parsers.DocumentBuilder;
import javax.xml.parsers.DocumentBuilderFactory;
import javax.xml.parsers.ParserConfigurationException;
import javax.xml.xpath.XPath;
import javax.xml.xpath.XPathConstants;
import javax.xml.xpath.XPathFactory;

import org.xml.sax.EntityResolver;
import org.xml.sax.InputSource;
import org.xml.sax.SAXException;

public class XmlDom {
    private static final String DISALLOW_DOCTYPE_DECL =
        "http://apache.org/xml/features/disallow-doctype-decl";
    private static final String EXTERNAL_GENERAL_ENTITIES =
        "http://xml.org/sax/features/external-general-entities";
    private static final String EXTERNAL_PARAMETER_ENTITIES =
        "http://xml.org/sax/features/external-parameter-entities";
    private static final String LOAD_EXTERNAL_DTD =
        "http://apache.org/xml/features/nonvalidating/load-external-dtd";

    private Document m_document;
    private XPath m_xpath;

    public XmlDom() {
        m_document = null;
        XPathFactory factory = XPathFactory.newInstance();
        m_xpath = factory.newXPath();
    }

    public void InitializeDocument(byte[] stream) throws Exception {
        InputStream is = new ByteArrayInputStream(stream);
        DocumentBuilderFactory factory = DocumentBuilderFactory.newInstance();
        SetFeatureIfSupported(factory, XMLConstants.FEATURE_SECURE_PROCESSING, true);
        SetFeatureIfSupported(factory, DISALLOW_DOCTYPE_DECL, true);
        SetFeatureIfSupported(factory, EXTERNAL_GENERAL_ENTITIES, false);
        SetFeatureIfSupported(factory, EXTERNAL_PARAMETER_ENTITIES, false);
        SetFeatureIfSupported(factory, LOAD_EXTERNAL_DTD, false);

        DocumentBuilder builder = factory.newDocumentBuilder();
        builder.setEntityResolver(new EntityResolver() {
            @Override
            public InputSource resolveEntity(String publicId, String systemId)
                    throws SAXException {
                throw new SAXException("External XML entities are not allowed");
            }
        });
        builder.setErrorHandler(new JavaXmlErrorHandler());
        Document document = builder.parse(is);
        if (document.getDoctype() != null) {
            throw new SAXException("DOCTYPE declarations are not allowed");
        }
        m_document = document;
    }

    private static void SetFeatureIfSupported(
            DocumentBuilderFactory factory, String feature, boolean value) {
        try {
            factory.setFeature(feature, value);
        } catch (ParserConfigurationException ignored) {
            // Android providers vary; the rejecting EntityResolver remains the fail-closed fallback.
        }
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