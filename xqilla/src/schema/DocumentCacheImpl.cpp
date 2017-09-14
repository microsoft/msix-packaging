/*
 * Copyright (c) 2001, 2008,
 *     DecisionSoft Limited. All rights reserved.
 * Copyright (c) 2004, 2015 Oracle and/or its affiliates. All rights reserved.
 *     
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "../config/xqilla_config.h"
#include <assert.h>

#include <xercesc/util/XMLURL.hpp>
#include <xercesc/util/XMLString.hpp>
#include <xercesc/util/XMLResourceIdentifier.hpp>
#include <xercesc/util/XMLEntityResolver.hpp>
#include <xercesc/framework/URLInputSource.hpp>
#include <xercesc/framework/LocalFileInputSource.hpp>
#include <xercesc/sax/SAXParseException.hpp>
#include <xercesc/validators/datatype/DatatypeValidatorFactory.hpp>
#include <xercesc/validators/schema/SchemaGrammar.hpp>
#include <xercesc/validators/schema/SchemaSymbols.hpp>
#include <xercesc/framework/XMLSchemaDescription.hpp>
#include <xercesc/framework/XMLGrammarPool.hpp>
#include <xercesc/internal/IGXMLScanner.hpp>

#include <xqilla/events/SequenceBuilder.hpp>
#include <xqilla/exceptions/DynamicErrorException.hpp>
#include <xqilla/exceptions/StaticErrorException.hpp>
#include <xqilla/exceptions/XMLParseException.hpp>
#include <xqilla/items/ATUntypedAtomic.hpp>
#include <xqilla/items/AnyAtomicType.hpp>
#include <xqilla/items/ATDurationOrDerived.hpp>
#include <xqilla/context/DynamicContext.hpp>
#include <xqilla/utils/XPath2Utils.hpp>
#include <xqilla/schema/AnyAtomicTypeDatatypeValidator.hpp>
#include <xqilla/schema/DocumentCacheImpl.hpp>
#include <xqilla/parser/QName.hpp>
#include <xqilla/items/DatatypeFactory.hpp>
#include <xqilla/context/ItemFactory.hpp>
#include <xqilla/events/QueryPathTreeFilter.hpp>
#include <xqilla/runtime/Sequence.hpp>

#include <xercesc/framework/psvi/PSVIElement.hpp>
#include <xercesc/framework/psvi/PSVIAttribute.hpp>
#include <xercesc/framework/psvi/PSVIAttributeList.hpp>

XERCES_CPP_NAMESPACE_USE;

class GrammarResolverHack : public XMemory
{
public:
  bool                            fCacheGrammar;
  bool                            fUseCachedGrammar;
  bool                            fGrammarPoolFromExternalApplication;
  XMLStringPool*                  fStringPool;
  RefHashTableOf<Grammar>*        fGrammarBucket;
  RefHashTableOf<Grammar>*        fGrammarFromPool;
  DatatypeValidatorFactory*       fDataTypeReg;
  MemoryManager*                  fMemoryManager;
  XMLGrammarPool*                 fGrammarPool;
  XSModel*                        fXSModel;
  XSModel*                        fGrammarPoolXSModel;
  ValueVectorOf<SchemaGrammar*>*  fGrammarsToAddToXSModel;
};

#if DEBUG
static int checkGrammarResolverHack()
{
	assert(sizeof(GrammarResolver) == sizeof(GrammarResolverHack));
	return 1;
}

static int checkGrammarResolverHack_int = checkGrammarResolverHack();
#endif

// untyped
const XMLCh DocumentCache::g_szUntyped[]= { chLatin_u, chLatin_n, chLatin_t, chLatin_y, chLatin_p, chLatin_e, chLatin_d, chNull };

DocumentCacheImpl::DocumentCacheImpl(MemoryManager* memMgr, XMLGrammarPool *xmlgr)
  : grammarResolver_(0),
    scanner_(0),
    entityResolver_(0),
    doPSVI_(true),
    events_(0),
    attrList_(0),
    attrCount_(0),
    elementInfo_(0),
    strictValidation_(false),
    loadedSchemas_(0),
    schemaLocations_(1023, memMgr),
    noNamespaceSchemaLocation_(1023, memMgr),
    memMgr_(memMgr)
{
  init(xmlgr);
}

DocumentCacheImpl::DocumentCacheImpl(const DocumentCacheImpl *parent, MemoryManager* memMgr)
  : grammarResolver_(0),
    scanner_(0),
    entityResolver_(0),
    doPSVI_(parent->doPSVI_),
    events_(0),
    attrList_(0),
    attrCount_(0),
    elementInfo_(0),
    strictValidation_(false),
    loadedSchemas_(0),
    schemaLocations_(1023, memMgr),
    noNamespaceSchemaLocation_(1023, memMgr),
    memMgr_(memMgr)
{
  init(parent->grammarResolver_->getGrammarPool());

  schemaLocations_.set(parent->schemaLocations_.getRawBuffer());
  noNamespaceSchemaLocation_.set(parent->noNamespaceSchemaLocation_.getRawBuffer());

  getScanner()->setExternalNoNamespaceSchemaLocation(noNamespaceSchemaLocation_.getRawBuffer());
  getScanner()->setExternalSchemaLocation(schemaLocations_.getRawBuffer());
}

void DocumentCacheImpl::init(XMLGrammarPool *gramPool)
{
  grammarResolver_ = new (memMgr_) GrammarResolver(gramPool, memMgr_);
  if(gramPool) {
    grammarResolver_->cacheGrammarFromParse(true);
    grammarResolver_->useCachedGrammarInParse(true);

    // Hack around a Xerces bug, where the GrammarResolver doesn't
    // initialise it's XSModel correctly from a locked XMLGrammarPool - jpcs

    // 2008/06/06 I don't think this is needed anymore - jpcs
//     ((GrammarResolverHack*)grammarResolver_)->fGrammarPoolXSModel = gramPool->getXSModel();
  }
}

XMLScanner *DocumentCacheImpl::getScanner()
{
  if(scanner_ == 0) {
    scanner_ = new (memMgr_) IGXMLScanner(0, grammarResolver_, memMgr_);
    scanner_->setURIStringPool(grammarResolver_->getStringPool());

    // hold the loaded schemas in the cache, so that can be reused    
    scanner_->cacheGrammarFromParse(true);
    scanner_->useCachedGrammarInParse(true);

    // set up the parser
    scanner_->setDocHandler(this);
    scanner_->setEntityHandler(this);
    scanner_->setErrorReporter(this);
    scanner_->setPSVIHandler(this);
    scanner_->setDoNamespaces(true);
    scanner_->setDoSchema(true);
    scanner_->setValidationScheme(XMLScanner::Val_Auto);
    scanner_->setValidationConstraintFatal(false);
  }
  return scanner_;
}

DocumentCacheImpl::~DocumentCacheImpl()
{
  delete scanner_;
  delete grammarResolver_;
  delete loadedSchemas_;
}

void DocumentCacheImpl::setXMLEntityResolver(XMLEntityResolver* const handler)
{
  entityResolver_ = handler;
}

XMLEntityResolver* DocumentCacheImpl::getXMLEntityResolver() const
{
  return entityResolver_;
}

void DocumentCacheImpl::startInputSource(const InputSource& inputSource)
{
}

void DocumentCacheImpl::endInputSource(const InputSource& inputSource)
{
}

bool DocumentCacheImpl::expandSystemId(const XMLCh* const systemId, XMLBuffer &toFill)
{
  return false;
}

void DocumentCacheImpl::resetEntities()
{
}

InputSource* DocumentCacheImpl::resolveEntity(const XMLCh* const publicId, const XMLCh* const systemId, const XMLCh* const baseURI)
{
  return 0;
}

InputSource* DocumentCacheImpl::resolveEntity(XMLResourceIdentifier* resourceIdentifier)
{
  if(entityResolver_)
    return entityResolver_->resolveEntity(resourceIdentifier);
  return 0;
}

void DocumentCacheImpl::resetErrors()
{
}

void DocumentCacheImpl::error(const unsigned int errCode, const XMLCh* const errDomain, const XMLErrorReporter::ErrTypes type, const XMLCh* const errorText,
                              const XMLCh* const systemId, const XMLCh* const publicId, const XMLFileLoc lineNum, const XMLFileLoc colNum)
{
  // Skip validation errors if validation isn't strict
  if(!strictValidation_ && errDomain == XMLUni::fgValidityDomain)
    return;

  // Throw for errors and fatal errors
  if(type != XMLErrorReporter::ErrType_Warning)
    throw SAXParseException(errorText, publicId, systemId, lineNum, colNum, memMgr_);
}

InputSource *DocumentCacheImpl::resolveURI(const XMLCh *uri, const XMLCh *baseUri)
{
  InputSource *srcToUse = 0;

  XMLURL urlTmp;
  if(urlTmp.setURL(baseUri, uri, urlTmp) && !urlTmp.isRelative()) {
    srcToUse = new URLInputSource(urlTmp);
  }
  else {
    // It's not a URL, so let's assume it's a local file name.
    AutoDeallocate<XMLCh> tmpBuf(0, XMLPlatformUtils::fgMemoryManager);
    if(baseUri && baseUri[0]) {
      tmpBuf.set(XMLPlatformUtils::weavePaths(baseUri, uri));
      uri = tmpBuf;
    }

    srcToUse = new LocalFileInputSource(uri);
  }

  return srcToUse;
}

Node::Ptr DocumentCacheImpl::loadDocument(const XMLCh* uri, DynamicContext *context, const QueryPathNode *projection)
{
  InputSource* srcToUse = 0;
  if(entityResolver_){
    XMLResourceIdentifier resourceIdentifier(XMLResourceIdentifier::UnKnown, uri, 0,
                                             XMLUni::fgZeroLenString, context->getBaseURI());
    srcToUse = entityResolver_->resolveEntity(&resourceIdentifier);
  }

  if(srcToUse == 0) {
    srcToUse = resolveURI(uri, context->getBaseURI());
  }

  Janitor<InputSource> janIS(srcToUse);

  return parseDocument(*srcToUse, context, projection);
}

Node::Ptr DocumentCacheImpl::parseDocument(InputSource &srcToUse, DynamicContext *context, const QueryPathNode *projection)
{
  AutoDelete<SequenceBuilder> builder(context->createSequenceBuilder());
  QueryPathTreeFilter qptf(projection, builder.get());
  EventHandler *handler = projection ? (EventHandler*)&qptf : (EventHandler*)builder.get();

  parseDocument(srcToUse, handler, context);

  handler->endEvent();
  return (Node*)builder->getSequence().first().get();
}

void DocumentCacheImpl::parseDocument(InputSource &srcToUse, EventHandler *handler, DynamicContext *context)
{
  events_ = handler;
  attrList_ = 0;
  attrCount_ = 0;
  elementInfo_ = 0;
  strictValidation_ = false;

  try {
    getScanner()->setDoSchema(doPSVI_);
    getScanner()->scanDocument(srcToUse);
  }
  catch(const SAXException& toCatch) {
    //TODO: Find a way to decipher whether the exception is actually because of a parsing problem or because the document can't be found
    XQThrow2(XMLParseException, X("DocumentCacheImpl::parseDocument"), toCatch.getMessage());
  }
  catch(const XMLException& toCatch) {
    XQThrow2(XMLParseException,X("DocumentCacheImpl::parseDocument"), toCatch.getMessage());
  }
}

static inline void setLocation(LocationInfo &info, const Locator *locator)
{
  info.setLocationInfo(locator->getSystemId(),
                       (unsigned int)(locator->getLineNumber()),
                       (unsigned int)(locator->getColumnNumber()));
}

#define LOCATION setLocation(location_, getScanner()->getLocator())

void DocumentCacheImpl::startDocument()
{
  LOCATION;
  events_->setLocationInfo(&location_);

  // Encode space chars in the document URI as %20
  const XMLCh *uri = getScanner()->getLocator()->getSystemId();

  XMLBuffer encode(XMLString::stringLen(uri) + 1);
  if(uri != 0) {
    for(const XMLCh *uptr = uri; *uptr; ++uptr) {
      if(*uptr != ' ')
        encode.append(*uptr);
      else {
        encode.append('%');
        encode.append('2');
        encode.append('0');
      }
    }
    uri = encode.getRawBuffer();
  }

  events_->startDocumentEvent(uri, getScanner()->getReaderMgr()->getCurrentEncodingStr());
}

void DocumentCacheImpl::endDocument()
{
  LOCATION;
  events_->endDocumentEvent();
}

void DocumentCacheImpl::resetDocument()
{
}

void DocumentCacheImpl::startElement(const XMLElementDecl& elemDecl, const unsigned int urlId,
                                     const XMLCh* const elemPrefix, const RefVectorOf<XMLAttr>& attrList,
                                     const XercesSizeUint attrCount, const bool isEmpty, const bool isRoot)
{
  LOCATION;
  events_->startElementEvent(emptyToNull(elemPrefix), emptyToNull(getScanner()->getURIText(urlId)), elemDecl.getBaseName());

  attrList_ = &attrList;
  attrCount_ = (unsigned int) attrCount;
  if(!getScanner()->getDoSchema() || getScanner()->getCurrentGrammarType() != Grammar::SchemaGrammarType) {
    handleAttributesPSVI(0, 0, 0);
  }

  if(isEmpty) {
	  endElement(elemDecl, urlId, isRoot, elemPrefix);
  }
}

void DocumentCacheImpl::endElement(const XMLElementDecl& elemDecl, const unsigned int urlId, const bool isRoot,
                                   const XMLCh* const elemPrefix)
{
  const XMLCh *typeURI = SchemaSymbols::fgURI_SCHEMAFORSCHEMA;
  const XMLCh *typeName = DocumentCache::g_szUntyped;

  if(elementInfo_ != 0 && elementInfo_->getValidity() == PSVIItem::VALIDITY_VALID) {
    if(elementInfo_->getMemberTypeDefinition()) {
      typeURI = elementInfo_->getMemberTypeDefinition()->getNamespace();
      typeName = elementInfo_->getMemberTypeDefinition()->getName();
    }
    else if(elementInfo_->getTypeDefinition()) {
      typeURI = elementInfo_->getTypeDefinition()->getNamespace();
      typeName = elementInfo_->getTypeDefinition()->getName();
    }
    else {
      typeURI = SchemaSymbols::fgURI_SCHEMAFORSCHEMA;
      typeName = SchemaSymbols::fgATTVAL_ANYTYPE;
    }
  }

  LOCATION;
  events_->endElementEvent(emptyToNull(elemPrefix), emptyToNull(getScanner()->getURIText(urlId)), elemDecl.getBaseName(),
                           emptyToNull(typeURI), typeName);

  elementInfo_ = 0;
}

void DocumentCacheImpl::docCharacters(const XMLCh* const chars, const XercesSizeUint length, const bool cdataSection)
{
  LOCATION;
  events_->textEvent(chars, (unsigned int)length);
}

void DocumentCacheImpl::docComment(const XMLCh* const comment)
{
  LOCATION;
  events_->commentEvent(comment);
}

void DocumentCacheImpl::docPI(const XMLCh* const target, const XMLCh* const data)
{
  LOCATION;
  events_->piEvent(target, data);
}

void DocumentCacheImpl::ignorableWhitespace(const XMLCh* const chars, const XercesSizeUint length,
                                            const bool cdataSection)
{
  // No-op
}

void DocumentCacheImpl::startEntityReference(const XMLEntityDecl& entDecl)
{
  // No-op
}

void DocumentCacheImpl::endEntityReference(const XMLEntityDecl& entDecl)
{
  // No-op
}

void DocumentCacheImpl::XMLDecl(const XMLCh* const versionStr, const XMLCh* const encodingStr,
                                const XMLCh* const standaloneStr, const XMLCh* const actualEncodingStr)
{
  // No-op
}

void DocumentCacheImpl::handleElementPSVI(const XMLCh* const localName, const XMLCh* const uri,
                                          PSVIElement *elementInfo)
{
  // We're going to deal with this in endElement(), where we know the prefix
  elementInfo_ = elementInfo;
}

void DocumentCacheImpl::handlePartialElementPSVI(const XMLCh* const localName, const XMLCh* const uri,
                                                 PSVIElement *elementInfo)
{
}

void DocumentCacheImpl::handleAttributesPSVI(const XMLCh* const localName, const XMLCh* const uri,
                                             PSVIAttributeList *psviAttributes)
{
  // We can't get any better location for an attribute than the element where is occured

  for(unsigned int i = 0; i < attrCount_; ++i) {
    const XMLAttr *attr = attrList_->elementAt(i);
    if(attr->getURIId() == getScanner()->getXMLNSNamespaceId()) {
      events_->namespaceEvent(attr->getName(), attr->getValue());
    }
    else if(XPath2Utils::equals(attr->getName(), XMLUni::fgXMLNSString)) {
      events_->namespaceEvent(0, attr->getValue());
    }
    else {
      const XMLCh *auri = getScanner()->getURIText(attr->getURIId());
      const XMLCh *typeURI = SchemaSymbols::fgURI_SCHEMAFORSCHEMA;
      const XMLCh *typeName = ATUntypedAtomic::fgDT_UNTYPEDATOMIC;

      if(psviAttributes != 0) {
        PSVIAttribute *attrInfo = psviAttributes->getAttributePSVIByName(attr->getName(), auri);
        if(attrInfo && attrInfo->getValidity() == PSVIItem::VALIDITY_VALID) {
          if(attrInfo->getMemberTypeDefinition()) {
            typeURI = attrInfo->getMemberTypeDefinition()->getNamespace();
            typeName = attrInfo->getMemberTypeDefinition()->getName();
          }
          else if(attrInfo->getTypeDefinition()) {
            typeURI = attrInfo->getTypeDefinition()->getNamespace();
            typeName = attrInfo->getTypeDefinition()->getName();
          }
          else {
            typeURI = SchemaSymbols::fgURI_SCHEMAFORSCHEMA;
            typeName = SchemaSymbols::fgDT_ANYSIMPLETYPE;
          }
        }
      }

      events_->attributeEvent(emptyToNull(attr->getPrefix()), emptyToNull(auri), attr->getName(), attr->getValue(),
                              emptyToNull(typeURI), typeName);
    }
  }

  attrList_ = 0;
  attrCount_ = 0;
}

static inline const XMLCh *nullToZeroLength(const XMLCh *in)
{
  return in ? in : XMLUni::fgZeroLenString;
}

/*
 * returns true if the type represented by uri:typename is an instance of uriToCheck:typeNameToCheck 
 *
 * ie: to check 
 * xs:integer instance of xs:decimal, 
 * call
 * isTypeOrDerivedFromType("xs", "integer", "xs", "decimal") 
 * (except of course, call with URIs, not prefixes!)
 */
bool DocumentCacheImpl::isTypeOrDerivedFromType(const XMLCh* const uri, const XMLCh* const typeName,
                                                const XMLCh* const uriToCheck,
                                                const XMLCh* const typeNameToCheck) const
{
  // dumb check
  if(XPath2Utils::equals(typeName,typeNameToCheck) && XPath2Utils::equals(uri,uriToCheck))
    return true;

  // xs:anyType matches anything
  if(XPath2Utils::equals(uriToCheck, SchemaSymbols::fgURI_SCHEMAFORSCHEMA) &&
     XPath2Utils::equals(typeNameToCheck, SchemaSymbols::fgATTVAL_ANYTYPE))
    return true;

  DatatypeValidator* dtvDerived = grammarResolver_->getDatatypeValidator(nullToZeroLength(uri), typeName);
  if(dtvDerived == NULL) {
    // now lets take a look at complex stuff
    ComplexTypeInfo *cti = getComplexTypeInfo(uri, typeName);

    // in the case of non-schema grammar
    if(cti) {
      // if we are here, the type is a complex type
      while(cti != 0) {
        if(XPath2Utils::equals(uriToCheck, cti->getTypeUri()) && 
           XPath2Utils::equals(typeNameToCheck, cti->getTypeLocalName())) 
          return true;
        ComplexTypeInfo *ctiOld = cti;
        cti = cti->getBaseComplexTypeInfo();

        // might be derived from a simple type
        if(cti == 0) {
          DatatypeValidator* dtv = ctiOld->getDatatypeValidator();

          while(dtv != 0) {
            if(XPath2Utils::equals(uriToCheck, dtv->getTypeUri()) && 
               XPath2Utils::equals(typeNameToCheck, dtv->getTypeLocalName())) 
              return true;
            dtv = dtv->getBaseValidator();
          }   
        }
      }
    }
    return false;
  }

  // if we are here, the type is a simple type
  if(XPath2Utils::equals(typeNameToCheck, SchemaSymbols::fgDT_ANYSIMPLETYPE) &&
     XPath2Utils::equals(uriToCheck, SchemaSymbols::fgURI_SCHEMAFORSCHEMA))
    return true;

  if(XPath2Utils::equals(typeNameToCheck, AnyAtomicType::fgDT_ANYATOMICTYPE) &&
     XPath2Utils::equals(uriToCheck, SchemaSymbols::fgURI_SCHEMAFORSCHEMA) )
    return dtvDerived->isAtomic();

  DatatypeValidator* dtvBase = grammarResolver_->getDatatypeValidator(nullToZeroLength(uriToCheck), typeNameToCheck);
  if(dtvBase==NULL)
    return false;
  while(dtvDerived != 0) {
    if(XPath2Utils::equals(dtvBase->getTypeUri(), dtvDerived->getTypeUri()) && 
       XPath2Utils::equals(dtvBase->getTypeLocalName(), dtvDerived->getTypeLocalName())) 
      return true;
    dtvDerived = dtvDerived->getBaseValidator();
  }

  return false;
}

bool DocumentCacheImpl::isTypeDefined(const XMLCh* const uri, const XMLCh* const typeName) const
{
  if(getComplexTypeInfo(uri, typeName) != NULL)
    return true;

  if(grammarResolver_->getDatatypeValidator(nullToZeroLength(uri), typeName) != NULL)
    return true;

  // these types are not present in the XMLSchema grammar, but they are defined
  if(XPath2Utils::equals(uri, SchemaSymbols::fgURI_SCHEMAFORSCHEMA)) {
    return (XPath2Utils::equals(typeName, DocumentCache::g_szUntyped) ||
            XPath2Utils::equals(typeName, SchemaSymbols::fgATTVAL_ANYTYPE));
  }
  return false;
}

void DocumentCacheImpl::addSchemaLocation(const XMLCh* uri, VectorOfStrings* locations, StaticContext *context, const LocationInfo *location)
{
  if(loadedSchemas_ == 0) {
    loadedSchemas_ = new (memMgr_) XMLStringPool(3, memMgr_);
  }

  XMLBuffer buf(1023,context->getMemoryManager());
  if(loadedSchemas_->exists(uri))
    {
      buf.set(X("More than one 'import schema' specifies the same target namespace \""));
      buf.append(uri);
      buf.append(X("\" [err:XQST0058]"));
      XQThrow3(StaticErrorException,X("DocumentCacheImpl::addSchemaLocation"), buf.getRawBuffer(), location);
    }
  loadedSchemas_->addOrFind(uri);

  bool bFoundSchema=false;
  if(locations==NULL) {
    // if no locations are given, try to see if the entity resolver can still find it
    try {
      loadSchema(uri, 0, context, location);
      bFoundSchema = true;
    } catch(SAXParseException& e) {
      buf.set(X("An error occurred while trying to load the schema for namespace \""));
      buf.append(uri);
      buf.append(X("\": "));
      buf.append(e.getMessage());
      buf.append(X(" [err:XQST0059]"));
    }
  }
  else {
    for(VectorOfStrings::iterator it=locations->begin(); it!=locations->end(); it++) {
      try {
        loadSchema(uri, *it, context, location);
        bFoundSchema = true;
        break;
      } catch(SAXParseException& e) {
        buf.set(X("An error occurred while trying to load the schema for namespace \""));
        buf.append(uri);
        buf.append(X("\" from \""));
        buf.append(*it);
        buf.append(X("\": "));
        buf.append(e.getMessage());
        buf.append(X(" [err:XQST0059]"));
      }
    }
  }
  if(!bFoundSchema) {
    if(buf.isEmpty()) {
      buf.set(X("Schema for namespace \""));
      buf.append(uri);
      buf.append(X("\" not found [err:XQST0059]"));
    }
    XQThrow3(StaticErrorException,X("DocumentCacheImpl::addSchemaLocation"), buf.getRawBuffer(), location);
  }
}

void DocumentCacheImpl::loadSchema(const XMLCh* const uri, const XMLCh* location, StaticContext *context, const LocationInfo *info)
{
  // if we are requested to load the XMLSchema schema, just return
  if(XPath2Utils::equals(uri, SchemaSymbols::fgURI_SCHEMAFORSCHEMA))
    return;
  InputSource* srcToUse = 0;
  if(entityResolver_){
    XMLResourceIdentifier resourceIdentifier(XMLResourceIdentifier::SchemaGrammar,
                                             location, uri, XMLUni::fgZeroLenString, 
                                             context->getBaseURI());
    srcToUse = entityResolver_->resolveEntity(&resourceIdentifier);
  }
  Janitor<InputSource> janIS(srcToUse);

  Grammar* grammar=NULL;
  if(srcToUse) {
    if(location == 0) location = srcToUse->getSystemId();
    grammar = getScanner()->loadGrammar(*srcToUse, Grammar::SchemaGrammarType, true);
  }
  else if(location) {
    // Resolve the location against the base uri
    const XMLCh *systemId = location;
    XMLURL urlTmp(context->getMemoryManager());
    if(urlTmp.setURL(context->getBaseURI(), location, urlTmp)) {
      systemId = urlTmp.getURLText();
    }

    grammar = getScanner()->loadGrammar(systemId, Grammar::SchemaGrammarType, true);
  }
  if(grammar==NULL)
    XQThrow3(StaticErrorException,X("DocumentCacheImpl::loadSchema"), X("Schema not found [err:XQST0059]"), info);

  // Update the scanner's external schema locations, so it validates
  // XML documents that match
  if(uri == 0 || *uri == 0) {
    noNamespaceSchemaLocation_.set(location);
    getScanner()->setExternalNoNamespaceSchemaLocation(noNamespaceSchemaLocation_.getRawBuffer());
  }
  else {
    schemaLocations_.append(' ');
    schemaLocations_.append(uri);
    schemaLocations_.append(' ');
    schemaLocations_.append(location);
    getScanner()->setExternalSchemaLocation(schemaLocations_.getRawBuffer());
  }
}

unsigned int DocumentCacheImpl::getSchemaUriId(const XMLCh* uri) const
{
  return grammarResolver_->getStringPool()->getId(uri);
}

const XMLCh* DocumentCacheImpl::getSchemaUri(unsigned int id) const
{
  return grammarResolver_->getStringPool()->getValueForId(id);
}

GrammarResolver *DocumentCacheImpl::getGrammarResolver() const
{
  return grammarResolver_;
}

DatatypeValidator*  DocumentCacheImpl::getDatatypeValidator(const XMLCh* uri, const XMLCh* typeName) const
{
  return grammarResolver_->getDatatypeValidator(nullToZeroLength(uri), typeName);
}

SchemaElementDecl* DocumentCacheImpl::getElementDecl(const XMLCh* elementUri, const XMLCh* elementName) const
{
  XMLSchemaDescription* gramDesc = grammarResolver_->getGrammarPool()->createSchemaDescription(elementUri);
  Janitor<XMLSchemaDescription> janName(gramDesc);
  SchemaGrammar* schGrammar = (SchemaGrammar*)grammarResolver_->getGrammar(gramDesc);
  if(schGrammar == NULL) return NULL;

  // find the definition for this node, as a global definition
  return (SchemaElementDecl*)schGrammar->getElemDecl(getSchemaUriId(elementUri), elementName, NULL,
                                                     (unsigned int)Grammar::TOP_LEVEL_SCOPE);
}

SchemaAttDef* DocumentCacheImpl::getAttributeDecl(const XMLCh* attributeUri, const XMLCh* attributeName) const
{
  XMLSchemaDescription* gramDesc = grammarResolver_->getGrammarPool()->createSchemaDescription(attributeUri);
  Janitor<XMLSchemaDescription> janName(gramDesc);
  SchemaGrammar* schGrammar = (SchemaGrammar*)grammarResolver_->getGrammar(gramDesc);
  if(schGrammar==NULL) return NULL;

  // find the definition for this node, as a global definition
  return (SchemaAttDef*)schGrammar->getAttributeDeclRegistry()->get(attributeName);
}

ComplexTypeInfo*  DocumentCacheImpl::getComplexTypeInfo(const XMLCh* uri, const XMLCh* typeName) const
{
  Grammar* grammar = grammarResolver_->getGrammar(uri);
  if(grammar==NULL) return NULL;
  if(grammar->getGrammarType() != Grammar::SchemaGrammarType) return NULL;

  SchemaGrammar* schGrammar = (SchemaGrammar*)grammar;
  RefHashTableOf<ComplexTypeInfo> *ctr = schGrammar->getComplexTypeRegistry();
  if(ctr==NULL) return NULL;

  XMLBuffer typeKey(1023, memMgr_);
  typeKey.set(uri);
  typeKey.append(chComma);
  typeKey.append(typeName);

  return ctr->get(typeKey.getRawBuffer());
}

DocumentCache *DocumentCacheImpl::createDerivedCache(MemoryManager *memMgr) const
{
  // lock the grammar pool, so we can share it across threads
  grammarResolver_->getGrammarPool()->lockPool();

  // Construct a new DocumentCacheImpl, based on this one
  return new (memMgr) DocumentCacheImpl(this, memMgr);
}
