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

/*
 * A large part of this file is directly derived from the
 * SGXMLScanner class in the Xerces-C project, which is 
 * licenced under the Apache License v2.0
 */

#include "../config/xqilla_config.h"
#include <xqilla/schema/SchemaValidatorFilter.hpp>
#include <xqilla/schema/DocumentCache.hpp>
#include <xqilla/items/ATUntypedAtomic.hpp>
#include <xqilla/utils/XPath2Utils.hpp>
#include <xqilla/context/DynamicContext.hpp>
#include <xqilla/exceptions/DynamicErrorException.hpp>

#include <xercesc/internal/XMLReader.hpp>
#include <xercesc/framework/MemBufInputSource.hpp>
#include <xercesc/validators/common/ContentLeafNameTypeVector.hpp>
#include <xercesc/validators/schema/SubstitutionGroupComparator.hpp>

using namespace std;
XERCES_CPP_NAMESPACE_USE;

SchemaValidatorFilter::SchemaValidatorFilter(bool strictValidation, EventHandler *next, GrammarResolver *grammarResolver,
                                             MemoryManager *mm, const LocationInfo *info)
  : EventFilter(next),
    XMLScanner(0, grammarResolver, mm),
    info_(info),
    fSchemaValidator(new (mm) SchemaValidator(0, mm)),
    fSchemaGrammar(0),
    fContent(1023, mm),
    fICHandler(0),
    fElemNonDeclPool(new (mm) RefHash3KeysIdPool<SchemaElementDecl>(29, true, 128, mm)),
    strictValidation_(strictValidation),
    errorOccurred_(false),
    parentStack_(0),
    elemDepth_(0),
    elementToProcess_(false),
    xsiType_(0),
    attrCount_(0)
{
  fSchemaGrammar = new (fGrammarPoolMemoryManager) SchemaGrammar(fGrammarPoolMemoryManager);
  fICHandler = new (fMemoryManager) IdentityConstraintHandler(this, fMemoryManager);

  // XMLScanner methods needed
  // ----------------------
  //   fScanner->incrementErrorCount();                 implemented
  //   fScanner->getValidationConstraintFatal();        implemented
  //   fScanner->getExitOnFirstFatal();                 implemented
  //   fScanner->getInException();                      implemented
  //   fScanner->getMemoryManager();                    implemented
  //   getScanner()->getEmptyNamespaceId();             implemented
  //   getScanner()->resolveQName();                    ***
  //   getScanner()->getURIText();                      implemented
  //   getScanner()->getDocHandler(); for docCharacters() ***
  //   getScanner()->getValidationContext();            implemented
  //   getScanner()->emitError();                       implemented
  //   getScanner()->getValidationSchemaFullChecking(); implemented

  setURIStringPool(grammarResolver->getStringPool());

  // ReaderMgr methods needed
  // ------------------------
  //   fReaderMgr->getLastExtEntityInfo();
  //   getReaderMgr()->getCurrentReader();  to call fCurReader->isWhitespace();

  MemBufInputSource inputSrc(0, 0, XMLUni::fgZeroLenString, false, fMemoryManager);
  XMLReader *newReader = fReaderMgr.createReader(inputSrc, true, XMLReader::RefFrom_NonLiteral, XMLReader::Type_General,
                                                 XMLReader::Source_External, /*fCalculateSrcOfs*/false);
  fReaderMgr.pushReader(newReader, 0);
  fReaderMgr.setXMLVersion(XMLReader::XMLV1_1);

  initValidator(fSchemaValidator);

  setDocHandler(this);
  setErrorReporter(this);

  reset();
}

SchemaValidatorFilter::~SchemaValidatorFilter()
{
  delete fSchemaValidator;
  delete fSchemaGrammar;
  delete fICHandler;
  delete fElemNonDeclPool;
}

void SchemaValidatorFilter::reset()
{
  fGrammar = fSchemaGrammar;
  fValidator = fSchemaValidator;
  fValidator->setGrammar(fGrammar);

  fValidate = true;

  fSchemaValidator->reset();
  fSchemaValidator->setErrorReporter(fErrorReporter);
  fSchemaValidator->setExitOnFirstFatal(true);
  fSchemaValidator->setGrammarResolver(fGrammarResolver);
  fICHandler->reset();
  fElemNonDeclPool->removeAll();

  errorOccurred_ = false;
}

void SchemaValidatorFilter::startDocumentEvent(const XMLCh *documentURI, const XMLCh *encoding)
{
  reset();
  next_->startDocumentEvent(documentURI, encoding);
}

void SchemaValidatorFilter::endDocumentEvent()
{
  if(fValidate) {
    checkIDRefs();
    fICHandler->endDocument();
  }

  next_->endDocumentEvent();
}

void SchemaValidatorFilter::startElementEvent(const XMLCh *prefix, const XMLCh *uri, const XMLCh *localname)
{
  if(elementToProcess_) processStartElement();

  prefix_.set(prefix);
  uri_.set(uri);
  localname_.set(localname);
  elementToProcess_ = true;
  attrCount_ = 0;
  xsiType_ = 0;

  if(fElemStack.isEmpty()) parentStack_ = 0;
  else parentStack_ = const_cast<ElemStack::StackElem*>(fElemStack.topElement());

  elemDepth_ = fElemStack.addLevel();
  fElemStack.setValidationFlag(fValidate);

  errorOccurred_ = false;

  fElemStack.addPrefix(prefix, fURIStringPool->addOrFind(uri));

  next_->startElementEvent(prefix, uri, localname);
}

void SchemaValidatorFilter::processStartElement()
{
  bool isRoot = parentStack_ == 0;
  unsigned int uriId = fURIStringPool->addOrFind(uri_.getRawBuffer());

  // Handle xsi:type
  if(xsiType_) {
    int colonPos = -1;
    unsigned int atUriId = resolveQName(xsiType_, fPrefixBuf, ElemStack::Mode_Element, colonPos);
    ((SchemaValidator*)fValidator)->setXsiType(fPrefixBuf.getRawBuffer(), xsiType_ + colonPos + 1, atUriId);
  }

  //if schema, check if we should lax or skip the validation of this element
  bool parentValidation = fValidate;
  unsigned int currentScope = Grammar::TOP_LEVEL_SCOPE;
  bool laxThisOne = false;
  if(!isRoot) {
    // schema validator will have correct type if validating
    SchemaElementDecl* tempElement = (SchemaElementDecl*)parentStack_->fThisElement;
    SchemaElementDecl::ModelTypes modelType = tempElement->getModelType();
    ComplexTypeInfo *currType = 0;

    if(fValidate) {
      currType = ((SchemaValidator*)fValidator)->getCurrentTypeInfo();
      if (currType)
        modelType = (SchemaElementDecl::ModelTypes)currType->getContentType();
      else // something must have gone wrong
        modelType = SchemaElementDecl::Any;
    }
    else {
      currType = tempElement->getComplexTypeInfo();
    }

    if(modelType == SchemaElementDecl::Mixed_Simple ||
       modelType == SchemaElementDecl::Mixed_Complex ||
       modelType == SchemaElementDecl::Children) {
      XMLContentModel* cm = currType->getContentModel();
      currentScope = parentStack_->fCurrentScope;

      QName element(prefix_.getRawBuffer(), localname_.getRawBuffer(), uriId, fMemoryManager);
      // elementDepth will be > 0
      laxThisOne = laxElementValidation(&element, cm->getContentLeafNameTypeVector(), cm, elemDepth_ - 1);
    }
    else if(modelType == SchemaElementDecl::Any) {
      laxThisOne = true;
    }
  }
  else {
    // Switch to the initial Grammar for the root element.
    // This is added to fix a bug which used fSchemaGrammar rather than the
    // real grammar when the document is in no namespace - jpcs
    switchGrammar(uri_.getRawBuffer());
  }

  XMLElementDecl* elemDecl = createElementDecl(uriId, currentScope, laxThisOne);
  assert(elemDecl);
  fElemStack.setElement(elemDecl, fReaderMgr.getCurrentReaderNum());

  if(fValidate) fValidator->validateElement(elemDecl);

  ComplexTypeInfo* typeinfo = fValidate ? ((SchemaValidator*)fValidator)->getCurrentTypeInfo() : ((SchemaElementDecl*)elemDecl)->getComplexTypeInfo();
  if(typeinfo) {
    currentScope = typeinfo->getScopeDefined();

    // switch grammar if the typeinfo has a different grammar (happens when there is xsi:type)
    if(!typeinfo->getAnonymous()) {
      int comma = XMLString::indexOf(typeinfo->getTypeName(), chComma);
      if(comma > 0) {
        XMLBuffer prefixBuf(comma + 1, fMemoryManager);
        prefixBuf.append(typeinfo->getTypeName(), comma);
        const XMLCh *uriStr = prefixBuf.getRawBuffer();

        if(!switchGrammar(uriStr) && fValidate && !laxThisOne) {
          fValidator->emitError(XMLValid::GrammarNotFound, prefixBuf.getRawBuffer());
        }
      }
      else if(comma == 0) {
        if(!switchGrammar(XMLUni::fgZeroLenString) && fValidate && !laxThisOne) {
          fValidator->emitError(XMLValid::GrammarNotFound, XMLUni::fgZeroLenString);
        }
      }
    }
  }
  fElemStack.setCurrentScope(currentScope);

  // We commandeer fCurrentURI to keep the element state in
  fElemStack.setCurrentURI(0);

  fElemStack.setCurrentGrammar(fGrammar);

  if(!isRoot && parentValidation) {
    fElemStack.addChild(elemDecl->getElementName(), true);
  }

  processAttrs(elemDecl);

  if(fValidate)
    fICHandler->activateIdentityConstraint((SchemaElementDecl*)elemDecl, (int)elemDepth_, uriId, prefix_.getRawBuffer(), *fAttrList, attrCount_);

  elementToProcess_ = false;

  // We commandeer fCommentOrPISeen to keep errorOccurred_ in
  if(errorOccurred_)
    fElemStack.setCommentOrPISeen();
}

void SchemaValidatorFilter::processAttrs(XMLElementDecl *elemDecl)
{
  DatatypeValidator *currDV = 0;
  ComplexTypeInfo *currType = 0;

  if(fValidate) {
    currType = ((SchemaValidator*)fValidator)->getCurrentTypeInfo();
    if(!currType)
      currDV = ((SchemaValidator*)fValidator)->getCurrentDatatypeValidator();
  }

//   XMLBufBid bbNormal(&fBufMgr);
//   XMLBuffer& normBuf = bbNormal.getBuffer();

  //  Loop through our explicitly provided attributes, which are in the raw
  //  scanned form, and build up XMLAttr objects.
  unsigned int index;
  for(index = 0; index < attrCount_; ++index) {
    const XMLAttr *attr = fAttrList->elementAt(index);
    const XMLCh *localname = attr->getName();
    const XMLCh *value = attr->getValue();

    unsigned int uriId = attr->getURIId();
    const XMLCh *uri = getURIText(uriId);

    bool attrValid = true;
    DatatypeValidator *attrValidator = 0;

    bool xsiAttr = false;
    if(XMLString::equals(uri, SchemaSymbols::fgURI_XSI)) {
      xsiAttr = true;

      if(XMLString::equals(localname, SchemaSymbols::fgATT_NILL)) {
        attrValidator = DatatypeValidatorFactory::getBuiltInRegistry()->get(SchemaSymbols::fgDT_BOOLEAN);
      }
      else if(XMLString::equals(localname, SchemaSymbols::fgXSI_SCHEMALOCACTION)) {
      }
      else if(XMLString::equals(localname, SchemaSymbols::fgXSI_NONAMESPACESCHEMALOCACTION)) {
      }
      else if(XMLString::equals(localname, SchemaSymbols::fgXSI_TYPE)) {
        attrValidator = DatatypeValidatorFactory::getBuiltInRegistry()->get(SchemaSymbols::fgDT_QNAME);
      }
      else {
        xsiAttr = false;
      }
    }

    if(xsiAttr) {
//         // Just normalize as CDATA
//         attType = XMLAttDef::CData;
//         normalizeAttRawValue
//           (
//            namePtr
//            , curPair->getValue()
//            , normBuf
//            );

      if(fValidate && attrValidator) {
        try {
          attrValidator->validate(value, fValidationContext, fMemoryManager);
        }
        catch (const XMLException& idve) {
          attrValid = false;
#if _XERCES_VERSION >= 20800
          fValidator->emitError (XMLValid::DatatypeError, idve.getCode(), idve.getType(), idve.getMessage());
#else
          fValidator->emitError (XMLValid::DatatypeError, idve.getType(), idve.getMessage());
#endif
        }
      }
    }
    else {
      // Some checking for attribute wild card first (for schema)
      bool laxThisOne = false;
      bool skipThisOne = false;

      XMLAttDef *attDef = 0;

      //retrieve the att def
      SchemaAttDef* attWildCard = 0;
      if(currType) {
        attDef = currType->getAttDef(localname, uriId);
        attWildCard = currType->getAttWildCard();
      }
      else if(!currDV) { // check explicitly-set wildcard
        attWildCard = ((SchemaElementDecl*)elemDecl)->getAttWildCard();
      }

      // if not found or faulted in - check for a matching wildcard attribute
      // if no matching wildcard attribute, check (un)qualifed cases and flag
      // appropriate errors
      if (!attDef || (attDef->getCreateReason() == XMLAttDef::JustFaultIn)) {

        if (attWildCard) {
          //if schema, see if we should lax or skip the validation of this attribute
          if (anyAttributeValidation(attWildCard, uriId, skipThisOne, laxThisOne)) {

            if(!skipThisOne)
            {
              SchemaGrammar* sGrammar = (SchemaGrammar*) fGrammarResolver->getGrammar(getURIText(uriId));
              if (sGrammar && sGrammar->getGrammarType() == Grammar::SchemaGrammarType) {
                RefHashTableOf<XMLAttDef>* attRegistry = sGrammar->getAttributeDeclRegistry();
                if (attRegistry) {
                  attDef = attRegistry->get(localname);
                }
              }
            }
          }
        }
        else if (currType) {
          // not found, see if the attDef should be qualified or not
          if (uriId == fEmptyNamespaceId) {
            attDef = currType->getAttDef(localname, fURIStringPool->getId(fGrammar->getTargetNamespace()));
            if (fValidate && attDef && attDef->getCreateReason() != XMLAttDef::JustFaultIn) {
              // the attribute should be qualified
              fValidator->emitError(XMLValid::AttributeNotQualified, attDef->getFullName());
              attrValid = false;
            }
          }
          else {
            attDef = currType->getAttDef(localname, fEmptyNamespaceId);
            if (fValidate && attDef && attDef->getCreateReason() != XMLAttDef::JustFaultIn) {
              // the attribute should be qualified
              fValidator->emitError(XMLValid::AttributeNotUnQualified, attDef->getFullName());
              attrValid = false;
            }
          }
        }
      }

      if(!attDef) attrValid = false;

      if(fValidate && !attDef && !skipThisOne && !laxThisOne) {
        //
        //  Its not valid for this element, so issue an error if we are
        //  validating.
        //
        XMLBufBid bbMsg(&fBufMgr);
        XMLBuffer& bufMsg = bbMsg.getBuffer();
        if(uriId != fEmptyNamespaceId) {
          XMLBufBid bbURI(&fBufMgr);
          XMLBuffer& bufURI = bbURI.getBuffer();

          getURIText(uriId, bufURI);

          bufMsg.append(chOpenCurly);
          bufMsg.append(bufURI.getRawBuffer());
          bufMsg.append(chCloseCurly);
        }
        bufMsg.append(localname);
        fValidator->emitError(XMLValid::AttNotDefinedForElement, bufMsg.getRawBuffer(), elemDecl->getFullName());
      }

      // TBD? - jpcs
//       normalizeAttValue(attDef, namePtr, value, normBuf);

      if(attDef) {
        DatatypeValidator* tempDV = ((SchemaAttDef*) attDef)->getDatatypeValidator();
        if(tempDV && tempDV->getWSFacet() != DatatypeValidator::PRESERVE) {
          // normalize the attribute according to schema whitespace facet
          ((SchemaValidator*) fValidator)->normalizeWhiteSpace(tempDV, value, fWSNormalizeBuf);
          value = fWSNormalizeBuf.getRawBuffer();
        }

        if(fValidate && !skipThisOne) {
          fValidator->validateAttrValue(attDef, value, false, elemDecl);
          attrValidator = ((SchemaValidator *)fValidator)->getMostRecentAttrValidator();
          if(((SchemaValidator *)fValidator)->getErrorOccurred())
            attrValid = false;
        }
      }
    }

    if(fValidate && attrValid) {
      if(attrValidator)
        next_->attributeEvent(emptyToNull(attr->getPrefix()), emptyToNull(uri), localname, value,
                              emptyToNull(attrValidator->getTypeUri()), attrValidator->getTypeLocalName());
      else 
        next_->attributeEvent(emptyToNull(attr->getPrefix()), emptyToNull(uri), localname, value, SchemaSymbols::fgURI_SCHEMAFORSCHEMA, SchemaSymbols::fgDT_ANYSIMPLETYPE);
    }
    else
      next_->attributeEvent(emptyToNull(attr->getPrefix()), emptyToNull(uri), localname, value, SchemaSymbols::fgURI_SCHEMAFORSCHEMA, ATUntypedAtomic::fgDT_UNTYPEDATOMIC);
  }

  // Deal with default, required and prohibited attrs
  if((currType && fValidate) ? currType->hasAttDefs() : elemDecl->hasAttDefs()) {

    XMLAttDefList &attDefList = currType ? currType->getAttDefList() : elemDecl->getAttDefList();
    for(unsigned int i = 0; i < attDefList.getAttDefCount(); ++i) {
      SchemaAttDef *curDef = (SchemaAttDef*)&attDefList.getAttDef(i);
      XMLAttDef::DefAttTypes defType = curDef->getDefaultType();
      QName* attName = curDef->getAttName();

      unsigned int curUriId = attName->getURI();
      const XMLCh *curName = attName->getLocalPart();

      // See if the attribute is already defined
      for(index = 0; index < attrCount_; ++index) {
        const XMLAttr *attr = fAttrList->elementAt(index);
        if(attr->getURIId() == curUriId &&
           XPath2Utils::equals(attr->getName(), curName))
          break;
      }

      if(index >= attrCount_) {
        switch(defType) {
        case XMLAttDef::Required:
        case XMLAttDef::Required_And_Fixed:
          if(fValidate)
            fValidator->emitError(XMLValid::RequiredAttrNotProvided, curDef->getFullName());
          break;
        case XMLAttDef::Default:
        case XMLAttDef::Fixed: {
          DatatypeValidator *attrValidator = 0;
          if(fValidate) {
            fValidator->validateAttrValue(curDef, curDef->getValue(), false, elemDecl);
            attrValidator = ((SchemaValidator *)fValidator)->getMostRecentAttrValidator();
          }

          if(attrValidator)
            next_->attributeEvent(emptyToNull(attName->getPrefix()), emptyToNull(getURIText(curUriId)), curName, curDef->getValue(),
                                  emptyToNull(attrValidator->getTypeUri()), attrValidator->getTypeLocalName());
          else
            next_->attributeEvent(emptyToNull(attName->getPrefix()), emptyToNull(getURIText(curUriId)), curName, curDef->getValue(),
                                  SchemaSymbols::fgURI_SCHEMAFORSCHEMA, ATUntypedAtomic::fgDT_UNTYPEDATOMIC);
          break;
        }
        default: break;
        }
      }
      else if(fValidate && defType == XMLAttDef::Prohibited) {
        fValidator->emitError(XMLValid::ProhibitedAttributePresent, curDef->getFullName());
      }
    }
  }
}

void SchemaValidatorFilter::endElementEvent(const XMLCh *prefix, const XMLCh *uri, const XMLCh *localname,
                                            const XMLCh *typeURI, const XMLCh *typeName)
{
  if(elementToProcess_) processStartElement();

  typeURI = SchemaSymbols::fgURI_SCHEMAFORSCHEMA;
  typeName = SchemaSymbols::fgATTVAL_ANYTYPE;

  assert(!fElemStack.isEmpty());
  const ElemStack::StackElem* topElem = fElemStack.topElement();

  // We commandeer fCommentOrPISeen to keep errorOccurred_ in
  errorOccurred_ = topElem->fCommentOrPISeen;

  if(fValidate) {
    DatatypeValidator *currentDV = 0;
    if(topElem->fThisElement->isDeclared()) {
      ComplexTypeInfo *currentTypeInfo = ((SchemaValidator*)fValidator)->getCurrentTypeInfo();
      if(currentTypeInfo) {
        typeURI = currentTypeInfo->getTypeUri();
        typeName = currentTypeInfo->getTypeLocalName();
      }
      else {
        currentDV = ((SchemaValidator*)fValidator)->getCurrentDatatypeValidator();
        typeURI = currentDV->getTypeUri();
        typeName = currentDV->getTypeLocalName();
      }
    }

#if _XERCES_VERSION >= 30000
    XMLSize_t failure;
    bool success = fValidator->checkContent(topElem->fThisElement, topElem->fChildren, topElem->fChildCount, &failure);
#else
    int res = fValidator->checkContent(topElem->fThisElement, topElem->fChildren, topElem->fChildCount);
    bool success = res == -1;
    XMLSize_t failure = (XMLSize_t)res;
#endif

    if(!success) {
      if(!topElem->fChildCount) {
        fValidator->emitError(XMLValid::EmptyNotValidForContent, topElem->fThisElement->getFormattedContentModel());
      }
      else if(failure >= topElem->fChildCount) {
        fValidator->emitError(XMLValid::NotEnoughElemsForCM, topElem->fThisElement->getFormattedContentModel());
      }
      else {
        fValidator->emitError(XMLValid::ElementNotValidForContent, topElem->fChildren[failure]->getRawName(), topElem->fThisElement->getFormattedContentModel());
      }
    }

    if(currentDV && currentDV->getType() == DatatypeValidator::Union) {
      DatatypeValidator *memberType = fValidationContext->getValidatingMemberType();
      typeURI = memberType->getTypeUri();
      typeName = memberType->getTypeLocalName();
    }

    if(fValidate)
      fICHandler->deactivateContext((SchemaElementDecl*)topElem->fThisElement, fContent.getRawBuffer());
  }

  if(!fValidate || errorOccurred_) {
    typeURI = SchemaSymbols::fgURI_SCHEMAFORSCHEMA;
    typeName = DocumentCache::g_szUntyped;
  }

  fElemStack.popTop(); 

  const bool isRoot = fElemStack.isEmpty();

  ((SchemaValidator *)fValidator)->clearDatatypeBuffer();

  if(!isRoot) {
    // We commandeer fCommentOrPISeen to keep errorOccurred_ in
    if(errorOccurred_)
      fElemStack.setCommentOrPISeen();

    fGrammar = fElemStack.getCurrentGrammar();
    fValidator->setGrammar(fGrammar);
    fValidate = fElemStack.getValidationFlag();
  }

  next_->endElementEvent(prefix, uri, localname, emptyToNull(typeURI), typeName);
}

static inline const XMLCh *nullToZero(const XMLCh *in)
{
  return in ? in : XMLUni::fgZeroLenString;
}

void SchemaValidatorFilter::attributeEvent(const XMLCh *prefix, const XMLCh *uri, const XMLCh *localname, const XMLCh *value,
                                           const XMLCh *typeURI, const XMLCh *typeName)
{
  unsigned int uriId = fURIStringPool->addOrFind(uri);
  fElemStack.addPrefix(prefix, uriId);

  XMLAttr *curAttr;
  if(attrCount_ >= fAttrList->size()) {
    curAttr = new (fMemoryManager) XMLAttr(uriId, localname, nullToZero(prefix), nullToZero(value), XMLAttDef::CData, true, fMemoryManager);
    fAttrList->addElement(curAttr);
  }
  else {
    curAttr = fAttrList->elementAt(attrCount_);
    curAttr->set(uriId, localname, nullToZero(prefix), nullToZero(value));
  }
  ++attrCount_;

  if(XPath2Utils::equals(uri, SchemaSymbols::fgURI_XSI)) {
    // Ignore schema location hints
    if(XPath2Utils::equals(localname, SchemaSymbols::fgXSI_TYPE)) {
      xsiType_ = curAttr->getValue();
    }
    else if(XPath2Utils::equals(localname, SchemaSymbols::fgATT_NILL) &&
            XPath2Utils::equals(value, SchemaSymbols::fgATTVAL_TRUE)) {
      ((SchemaValidator*)fValidator)->setNillable(true);
    }
  }
}

void SchemaValidatorFilter::namespaceEvent(const XMLCh *prefix, const XMLCh *uri)
{
  fElemStack.addPrefix(prefix, fURIStringPool->addOrFind(uri));

  next_->namespaceEvent(prefix, uri);
}

void SchemaValidatorFilter::textEvent(const XMLCh *chars)
{
  if(elementToProcess_) processStartElement();

  const XMLCh *end = chars;
  while(*end) ++end;
  unsigned int len = (unsigned int)(end - chars);

  if(fValidate) {
    // Get the character data opts for the current element
    XMLElementDecl::CharDataOpts charOpts = XMLElementDecl::AllCharData;
    ComplexTypeInfo *currType = ((SchemaValidator*)fValidator)->getCurrentTypeInfo();
    if(currType) {
      SchemaElementDecl::ModelTypes modelType = (SchemaElementDecl::ModelTypes)currType->getContentType(); 
      if(modelType == SchemaElementDecl::Children) 
        charOpts = XMLElementDecl::SpacesOk;
      else if(modelType == SchemaElementDecl::Empty) 
        charOpts = XMLElementDecl::NoCharData; 
    } 

    if(charOpts == XMLElementDecl::AllCharData) {
      DatatypeValidator *tempDV = ((SchemaValidator*) fValidator)->getCurrentDatatypeValidator();
      if(tempDV && tempDV->getWSFacet() != DatatypeValidator::PRESERVE) {
        // normalize the character according to schema whitespace facet
        ((SchemaValidator*) fValidator)->normalizeWhiteSpace(tempDV, chars, fWSNormalizeBuf);
        chars = fWSNormalizeBuf.getRawBuffer();
        len = (unsigned int) fWSNormalizeBuf.getLen();
      }

      // tell the schema validation about the character data for checkContent later
      ((SchemaValidator*)fValidator)->setDatatypeBuffer(chars);
    }
    else if(charOpts == XMLElementDecl::NoCharData) {
      fValidator->emitError(XMLValid::NoCharDataInCM);
    }
    else if(!fReaderMgr.getCurrentReader()->isAllSpaces(chars, len)) {
      fValidator->emitError(XMLValid::NoCharDataInCM);
    }
  }

  // call all active identity constraints
  if(fValidate && fICHandler->getMatcherCount())
    fContent.append(chars, len);

  next_->textEvent(chars, len);
}

void SchemaValidatorFilter::textEvent(const XMLCh *chars, unsigned int len)
{
  // Copy the chars to null terminate them
  XMLBuffer nullterm;
  nullterm.set(chars, len);
  textEvent(nullterm.getRawBuffer());
}

void SchemaValidatorFilter::commentEvent(const XMLCh *value)
{
  if(elementToProcess_) processStartElement();

  next_->commentEvent(value);
}

void SchemaValidatorFilter::piEvent(const XMLCh *target, const XMLCh *value)
{
  if(elementToProcess_) processStartElement();

  next_->piEvent(target, value);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

#if _XERCES_VERSION < 30100

unsigned int SchemaValidatorFilter::resolveQName(const XMLCh *const qName, XMLBuffer &prefixBuf,
                                                 const short mode, int &prefixColonPos)
{
  prefixColonPos = XMLString::indexOf(qName, chColon);

  if(prefixColonPos == -1) {
    bool unknown = false;
    return fElemStack.mapPrefixToURI(XMLUni::fgZeroLenString, (ElemStack::MapModes) mode, unknown);
  }
  else {
    prefixBuf.set(qName, prefixColonPos);

    if(XPath2Utils::equals(prefixBuf.getRawBuffer(), XMLUni::fgXMLNSString)) {
      if(mode == ElemStack::Mode_Element)
        emitError(XMLErrs::NoXMLNSAsElementPrefix, qName);

      return fXMLNSNamespaceId;
    }
    else if(XPath2Utils::equals(prefixBuf.getRawBuffer(), XMLUni::fgXMLString)) {
      return  fXMLNamespaceId;
    }
    else {
      bool unknown = false;
      unsigned int uriId = fElemStack.mapPrefixToURI(prefixBuf.getRawBuffer(), (ElemStack::MapModes) mode, unknown);

      if(unknown)
        emitError(XMLErrs::UnknownPrefix, prefixBuf.getRawBuffer());

      return uriId;
    }
  }
}

#endif

bool SchemaValidatorFilter::switchGrammar(const XMLCh *uri)
{
  Grammar* tempGrammar = fGrammarResolver->getGrammar(uri);

  if(!tempGrammar) tempGrammar = fSchemaGrammar;

  if(!tempGrammar || tempGrammar->getGrammarType() == Grammar::DTDGrammarType) return false;

  fGrammar = tempGrammar;
  fValidator->setGrammar(fGrammar);
  return true;
}

bool SchemaValidatorFilter::laxElementValidation(QName* element, ContentLeafNameTypeVector* cv,
                                                 const XMLContentModel* cm,
                                                 unsigned int parentElemDepth)
{
  bool skipThisOne = false;
  bool laxThisOne = false;
  unsigned int elementURI = element->getURI();

  // We commandeer fCurrentURI to keep the element state in
  unsigned int currState = parentStack_->fCurrentURI;

  if (currState == XMLContentModel::gInvalidTrans) {
    return laxThisOne;
  }

  SubstitutionGroupComparator comparator(fGrammarResolver, fURIStringPool);

  if(cv) {
    unsigned int i = 0;
    unsigned int leafCount = (unsigned int) cv->getLeafCount();

    for(; i < leafCount; ++i) {
      QName* fElemMap = cv->getLeafNameAt(i);
      unsigned int uri = fElemMap->getURI();
      unsigned int nextState;
      bool anyEncountered = false;
      ContentSpecNode::NodeTypes type = cv->getLeafTypeAt(i);

      if(type == ContentSpecNode::Leaf) {
        if(((uri == elementURI)
            && XMLString::equals(fElemMap->getLocalPart(), element->getLocalPart()))
           || comparator.isEquivalentTo(element, fElemMap)) {

          nextState = cm->getNextState(currState, i);

          if(nextState != XMLContentModel::gInvalidTrans) {
            // We commandeer fCurrentURI to keep the element state in
            parentStack_->fCurrentURI = nextState;
            break;
          }
        }
      } else if((type & 0x0f) == ContentSpecNode::Any) {
        anyEncountered = true;
      }
      else if((type & 0x0f) == ContentSpecNode::Any_Other) {
        if (uri != elementURI) {
          anyEncountered = true;
        }
      }
      else if((type & 0x0f) == ContentSpecNode::Any_NS) {
        if (uri == elementURI) {
          anyEncountered = true;
        }
      }

      if(anyEncountered) {

        nextState = cm->getNextState(currState, i);
        if(nextState != XMLContentModel::gInvalidTrans) {
          // We commandeer fCurrentURI to keep the element state in
          parentStack_->fCurrentURI = nextState;

          if(type == ContentSpecNode::Any_Skip ||
             type == ContentSpecNode::Any_NS_Skip ||
             type == ContentSpecNode::Any_Other_Skip) {
            skipThisOne = true;
          }
          else if(type == ContentSpecNode::Any_Lax ||
                  type == ContentSpecNode::Any_NS_Lax ||
                  type == ContentSpecNode::Any_Other_Lax) {
            laxThisOne = true;
          }

          break;
        }
      }
    }

    if(i == leafCount) { // no match
      // We commandeer fCurrentURI to keep the element state in
      parentStack_->fCurrentURI = XMLContentModel::gInvalidTrans;
      return laxThisOne;
    }

  }

  if (skipThisOne) {
    fValidate = false;
    fElemStack.setValidationFlag(fValidate);
  }

  return laxThisOne;
}

bool SchemaValidatorFilter::anyAttributeValidation(SchemaAttDef* attWildCard, unsigned int uriId, bool& skipThisOne, bool& laxThisOne)
{
  XMLAttDef::AttTypes wildCardType = attWildCard->getType();
  bool anyEncountered = false;
  skipThisOne = false;
  laxThisOne = false;
  if (wildCardType == XMLAttDef::Any_Any)
    anyEncountered = true;
  else if (wildCardType == XMLAttDef::Any_Other) {
    if (attWildCard->getAttName()->getURI() != uriId
        && uriId != fEmptyNamespaceId)
      anyEncountered = true;
  }
  else if (wildCardType == XMLAttDef::Any_List) {
    ValueVectorOf<unsigned int>* nameURIList = attWildCard->getNamespaceList();
    unsigned int listSize = (unsigned int) ((nameURIList) ? nameURIList->size() : 0);

    if (listSize) {
      for (unsigned int i=0; i < listSize; i++) {
        if (nameURIList->elementAt(i) == uriId)
          anyEncountered = true;
      }
    }
  }

  if (anyEncountered) {
    XMLAttDef::DefAttTypes   defType   = attWildCard->getDefaultType();
    if (defType == XMLAttDef::ProcessContents_Skip) {
      // attribute should just be bypassed,
      skipThisOne = true;
    }
    else if (defType == XMLAttDef::ProcessContents_Lax) {
      laxThisOne = true;
    }
  }

  return anyEncountered;
}

XMLElementDecl *SchemaValidatorFilter::createElementDecl(unsigned int uriId, unsigned int currentScope, bool laxThisOne)
{
  //  Look up the element now in the grammar. This will get us back a
  //  generic element decl object. We tell him to fault one in if he does
  //  not find it.
  XMLElementDecl* elemDecl = 0;
  const XMLCh* original_uriStr = fGrammar->getTargetNamespace();

  elemDecl = fGrammar->getElemDecl(uriId, localname_.getRawBuffer(), 0, currentScope);
  if(!elemDecl)
    elemDecl = fElemNonDeclPool->getByKey(localname_.getRawBuffer(), uriId, currentScope);

  unsigned int orgGrammarUri = uriId;
  if(!elemDecl && (orgGrammarUri = fURIStringPool->getId(original_uriStr)) != uriId) {
    //not found, switch grammar and try globalNS
    if(!switchGrammar(uri_.getRawBuffer()) && fValidate && !laxThisOne) {
      fValidator->emitError(XMLValid::GrammarNotFound, uri_.getRawBuffer());
    }
    elemDecl = fGrammar->getElemDecl(uriId, localname_.getRawBuffer(), 0, currentScope);
  }

  if(!elemDecl && currentScope != Grammar::TOP_LEVEL_SCOPE) {
    // if not found, then it may be a reference, try TOP_LEVEL_SCOPE
    elemDecl = fGrammar->getElemDecl(uriId, localname_.getRawBuffer(), 0, Grammar::TOP_LEVEL_SCOPE);
    if(!elemDecl)
      elemDecl = fElemNonDeclPool->getByKey(localname_.getRawBuffer(), uriId, Grammar::TOP_LEVEL_SCOPE);

    if(!elemDecl) {
      // still not found in specified uri
      if(uriId != fEmptyNamespaceId) {
        // try emptyNamesapce see if element should be un-qualified.
        elemDecl = fGrammar->getElemDecl(fEmptyNamespaceId, localname_.getRawBuffer(), 0, currentScope); 
        if(elemDecl && elemDecl->getCreateReason() != XMLElementDecl::JustFaultIn && fValidate) {
          fValidator->emitError(XMLValid::ElementNotUnQualified, elemDecl->getFullName());
        }
      }
      else if(orgGrammarUri != uriId) {
        // go to original Grammar again to see if element needs to be fully qualified.
        if(!switchGrammar(original_uriStr) && fValidate && !laxThisOne) {
          fValidator->emitError(XMLValid::GrammarNotFound, original_uriStr);
        }

        elemDecl = fGrammar->getElemDecl(orgGrammarUri, localname_.getRawBuffer(), 0, currentScope);
        if(elemDecl && elemDecl->getCreateReason() != XMLElementDecl::JustFaultIn && fValidate) {
          fValidator->emitError(XMLValid::ElementNotQualified, elemDecl->getFullName());
        }
      }
    }
  }

  if(!elemDecl) {
    // still not found, fault this in and issue error later
    // switch back to original grammar first (if necessary)
    if(orgGrammarUri != uriId) {
      switchGrammar(original_uriStr);
    }
    elemDecl = new (fMemoryManager) SchemaElementDecl(prefix_.getRawBuffer(), localname_.getRawBuffer(), uriId, SchemaElementDecl::Any,
                                                      Grammar::TOP_LEVEL_SCOPE, fMemoryManager);
    elemDecl->setId(fElemNonDeclPool->put((void*)elemDecl->getBaseName(), uriId, currentScope, (SchemaElementDecl*)elemDecl));

    if(laxThisOne) {
      fValidate = false;
      fElemStack.setValidationFlag(fValidate);
    }
    else if(fValidate) {
      // This is to tell the reuse Validator that this element was
      // faulted-in, was not an element in the grammar pool originally
      elemDecl->setCreateReason(XMLElementDecl::JustFaultIn);

      fValidator->emitError(XMLValid::ElementNotDefined, elemDecl->getFullName());
    }
  }
  else {
    // If its not marked declared and validating, then emit an error
    if(!elemDecl->isDeclared()) {
      if(laxThisOne) {
        fValidate = false;
        fElemStack.setValidationFlag(fValidate);                
      }
      else if(fValidate) {
        fValidator->emitError(XMLValid::ElementNotDefined, elemDecl->getFullName());
      }
    }
  }

  return elemDecl;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void SchemaValidatorFilter::docCharacters(const XMLCh* const chars, const XercesSizeUint length, const bool cdataSection)
{
  // The SchemaValidator calls this method to report default element values
  next_->textEvent(chars, (unsigned int)length);
}

void SchemaValidatorFilter::error(const unsigned int errCode, const XMLCh* const errDomain, const XMLErrorReporter::ErrTypes type, const XMLCh* const errorText,
                                  const XMLCh* const systemId, const XMLCh* const publicId, const XMLFileLoc lineNum, const XMLFileLoc colNum)
{
  errorOccurred_ = true;

  // Skip validation errors if validation isn't strict
  if(!strictValidation_ && errDomain == XMLUni::fgValidityDomain &&
     type != XMLErrorReporter::ErrType_Fatal)
    return;

  XMLBuffer exc_msg(1023);
  exc_msg.set(X("Schema validation failed: "));
  exc_msg.append(errorText);

  if((publicId && *publicId) || (systemId && *systemId)) {
    exc_msg.append(X(", "));
    if(systemId && *systemId)
      exc_msg.append(systemId);
    else
      exc_msg.append(publicId);
    if(lineNum != 0) {
      exc_msg.append(':');
      XPath2Utils::numToBuf((unsigned int)lineNum, exc_msg);
      exc_msg.append(':');
      XPath2Utils::numToBuf((unsigned int)colNum, exc_msg);
    }
  }

  exc_msg.append(X(" [err:XQDY0027]"));
  XQThrow3(DynamicErrorException,X("SchemaValidatorFilter::error"), exc_msg.getRawBuffer(), info_);
}
