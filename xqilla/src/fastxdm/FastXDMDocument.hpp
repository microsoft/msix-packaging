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

#ifndef _FASTXDMDOCUMENT_HPP
#define _FASTXDMDOCUMENT_HPP

#include <xqilla/framework/XQillaExport.hpp>
#include <xqilla/framework/ReferenceCounted.hpp>

#include <xqilla/events/EventHandler.hpp>

#include <xercesc/util/ValueStackOf.hpp>
#include <xercesc/framework/XMLBuffer.hpp>

class XPath2MemoryManager;

/// Stores and manages all the information for a Fast XDM document
class XQILLA_API FastXDMDocument : public EventHandler, public ReferenceCounted
{
public:
  typedef RefCountPointer<FastXDMDocument> Ptr;

  enum NodeKind {
    DOCUMENT,
    ELEMENT,
    TEXT,
    COMMENT,
    PROCESSING_INSTRUCTION,
    MARKER
  };

  struct Node;

  struct Attribute {
    void set(unsigned int oi, const XMLCh *p, const XMLCh *u, const XMLCh *l, const XMLCh *v, const XMLCh *tu, const XMLCh *tn)
    {
      owner.index = oi; prefix = p; uri = u; localname = l; value = v; typeURI = tu; typeName = tn;
    }
    void setMarker()
    {
      owner.index = (unsigned int)-1; prefix = 0; uri = 0; localname = 0; value = 0;
    }

    union {
      unsigned int index;
      Node *ptr;
    } owner;

    const XMLCh *prefix;
    const XMLCh *uri;
    const XMLCh *localname;

    const XMLCh *value;

    const XMLCh *typeURI;
    const XMLCh *typeName;
  };

  struct Namespace {
    void set(unsigned int oi, const XMLCh *p, const XMLCh *u)
    {
      owner.index = oi; prefix = p; uri = u;
    }
    void setMarker()
    {
      owner.index = (unsigned int)-1; prefix = 0; uri = 0;
    }

    union {
      unsigned int index;
      Node *ptr;
    } owner;

    const XMLCh *prefix;
    const XMLCh *uri;
  };

  struct Node {
    void setDocument(const XMLCh *d, const XMLCh *e)
    {
      nodeKind = DOCUMENT; level = 0; nextSibling.index = (unsigned int)-1; data.document.documentURI = d;
      data.document.encoding = e;
    }
    void setElement(unsigned int lv, const XMLCh *p, const XMLCh *u, const XMLCh *l)
    {
      nodeKind = ELEMENT; level = lv; nextSibling.index = (unsigned int)-1;
      data.element.attributes.index = (unsigned int)-1; data.element.namespaces.index = (unsigned int)-1;
      data.element.prefix = p; data.element.uri = u; data.element.localname = l;
    }
    void setElementType(const XMLCh *tu, const XMLCh *tn)
    {
      data.element.typeURI = tu; data.element.typeName = tn;
    }
    void setPI(unsigned int lv, const XMLCh *t, const XMLCh *v)
    {
      nodeKind = PROCESSING_INSTRUCTION; level = lv; nextSibling.index = (unsigned int)-1; data.other.target = t;
      data.other.value = v;
    }
    void setText(unsigned int lv, const XMLCh *v)
    {
      nodeKind = TEXT; level = lv; nextSibling.index = (unsigned int)-1; data.other.target = 0; data.other.value = v;
    }
    void setComment(unsigned int lv, const XMLCh *v)
    {
      nodeKind = COMMENT; level = lv; nextSibling.index = (unsigned int)-1; data.other.target = 0; data.other.value = v;
    }
    void setMarker()
    {
      nodeKind = MARKER; level = 0; nextSibling.index = (unsigned int)-1;
    }

    NodeKind nodeKind;
    unsigned int level;

    union {
      unsigned int index;
      Node *ptr;
    } nextSibling;

    union {
      struct {
        const XMLCh *documentURI;
        const XMLCh *encoding;
      } document;

      struct {
        union {
          unsigned int index;
          Attribute *ptr;
        } attributes;

        union {
          unsigned int index;
          Namespace *ptr;
        } namespaces;

        const XMLCh *prefix;
        const XMLCh *uri;
        const XMLCh *localname;

        const XMLCh *typeURI;
        const XMLCh *typeName;
      } element;

      struct {
        const XMLCh *target;
        const XMLCh *value;
      } other;

    } data;

  };

  FastXDMDocument(XPath2MemoryManager *mm);
  FastXDMDocument(unsigned int numNodes, unsigned int numAttributes, unsigned int numNamespaces, XPath2MemoryManager *mm);
  virtual ~FastXDMDocument();

  virtual void startDocumentEvent(const XMLCh *documentURI, const XMLCh *encoding);
  virtual void endDocumentEvent();
  virtual void startElementEvent(const XMLCh *prefix, const XMLCh *uri, const XMLCh *localname);
  virtual void endElementEvent(const XMLCh *prefix, const XMLCh *uri, const XMLCh *localname,
                               const XMLCh *typeURI, const XMLCh *typeName);
  virtual void piEvent(const XMLCh *target, const XMLCh *value);
  virtual void textEvent(const XMLCh *value);
  virtual void textEvent(const XMLCh *chars, unsigned int length);
  virtual void commentEvent(const XMLCh *value);
  virtual void attributeEvent(const XMLCh *prefix, const XMLCh *uri, const XMLCh *localname, const XMLCh *value,
                              const XMLCh *typeURI, const XMLCh *typeName);
  virtual void namespaceEvent(const XMLCh *prefix, const XMLCh *uri);
  virtual void endEvent();

  Node *getNode(unsigned int i);
  const Node *getNode(unsigned int i) const;
  unsigned int getNumNodes() const { return numNodes_; }

  Attribute *getAttribute(unsigned int i);
  const Attribute *getAttribute(unsigned int i) const;
  unsigned int getNumAttributes() const { return numAttributes_; }

  Namespace *getNamespace(unsigned int i);
  const Namespace *getNamespace(unsigned int i) const;
  unsigned int getNumNamespaces() const { return numNamespaces_; }

  static const Node *getParent(const Node *node);

  static void toEvents(const Node *node, EventHandler *events, bool preserveNS = true, bool preserveType = true);
  static void toEvents(const Attribute *attr, EventHandler *events, bool preserveType = true);
  static void toEvents(const Namespace *ns, EventHandler *events);

private:
  void resizeNodes();
  void resizeAttributes();
  void resizeNamespaces();

  XERCES_CPP_NAMESPACE_QUALIFIER ValueStackOf<unsigned int> elementStack_;
  unsigned int prevNode_;

  XERCES_CPP_NAMESPACE_QUALIFIER XMLBuffer textBuffer_;
  bool textToCreate_;

  Node *nodes_;
  unsigned int numNodes_;
  unsigned int maxNodes_;

  Attribute *attributes_;
  unsigned int numAttributes_;
  unsigned int maxAttributes_;

  Namespace *namespaces_;
  unsigned int numNamespaces_;
  unsigned int maxNamespaces_;

  XPath2MemoryManager *mm_;
};

inline const FastXDMDocument::Node *FastXDMDocument::getParent(const Node *node)
{
  unsigned int level = node->level;

  if(level == 0) return 0;

  do {
    --node;
  } while(node->level >= level);

  return node;
}

#endif


