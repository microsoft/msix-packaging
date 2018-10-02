//
//  Copyright (C) 2017 Microsoft.  All rights reserved.
//  See LICENSE file in the project root for full license information.
// 
#import <Foundation/Foundation.h>

@interface NSXmlParserDelegateWrapper : NSObject <NSXMLParserDelegate>
    -(id) initWithXmlDocumentReader:(void *) xmlDocumentReader;
@end
