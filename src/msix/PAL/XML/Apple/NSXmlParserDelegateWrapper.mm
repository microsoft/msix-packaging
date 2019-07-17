//
//  Copyright (C) 2017 Microsoft.  All rights reserved.
//  See LICENSE file in the project root for full license information.
// 
#import "NSXmlParserDelegateWrapper.h"
#import "XmlDocumentReader.hpp"

@implementation NSXmlParserDelegateWrapper{
    MSIX::XmlDocumentReader* m_xmlDocumentReader;
}

- (id) initWithXmlDocumentReader:(void *)xmlDocumentReader{
    self = [super init];
    if (self)
    {
        m_xmlDocumentReader = static_cast<MSIX::XmlDocumentReader*>(xmlDocumentReader);
    }
    return self;
}

- (void) parserDidStartDocument:(NSXMLParser *)parser {
}

- (void)parser:(NSXMLParser *)parser didStartElement:(NSString *)elementName namespaceURI:(NSString *)namespaceURI qualifiedName:(NSString *)qName attributes:(NSDictionary *)attributeDict {
    std::unique_ptr<MSIX::XmlNode> node(new MSIX::XmlNode());

    node->NodeName = std::string([elementName UTF8String]);
    std::size_t semiColon = node->NodeName.find_first_of(':');
    if (semiColon != std::string::npos)
    {
        node->NodeName = node->NodeName.substr(semiColon + 1);
    }
    if (qName)
    {
        node->QualifiedNodeName = std::string([qName UTF8String]);
    }
    for(id key in attributeDict)
    {
        node->Attributes.emplace(std::string([key UTF8String]), std::string([[attributeDict objectForKey:key] UTF8String]));
    }
    m_xmlDocumentReader->ProcessNodeBegin(std::move(node));
}

-(void) parser:(NSXMLParser *)parser foundCharacters:(NSString *)string {
    m_xmlDocumentReader->ProcessCharacters([string UTF8String]);
}

- (void)parser:(NSXMLParser *)parser didEndElement:(NSString *)elementName namespaceURI:(NSString *)namespaceURI qualifiedName:(NSString *)qName {
    std::string name = std::string([elementName UTF8String]);
    std::size_t semiColon = name.find_first_of(':');
    if (semiColon != std::string::npos)
    {
        name = name.substr(semiColon + 1);
    }
     m_xmlDocumentReader->ProcessNodeEnd(name);
}

- (void) parserDidEndDocument:(NSXMLParser *)parser {
}

- (void) parser:(NSXMLParser *)parser parseErrorOccurred:(NSError *)parseError {
}
@end