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

#include <xqilla/simple-api/XQilla.hpp>
#include <xqilla/simple-api/XQQuery.hpp>
#include <xqilla/context/DynamicContext.hpp>
#include <xqilla/context/ItemFactory.hpp>
#include <xqilla/exceptions/XQException.hpp>
#include <xqilla/utils/XStr.hpp>
#include <xqilla/utils/UTF8Str.hpp>
#include <xqilla/framework/XPath2MemoryManager.hpp>
#include <xqilla/ast/LocationInfo.hpp>
#include <xqilla/context/MessageListener.hpp>
#include <xqilla/events/EventSerializer.hpp>
#include <xqilla/events/NSFixupFilter.hpp>
#include <xqilla/xerces/XercesConfiguration.hpp>
#include <xqilla/fastxdm/FastXDMConfiguration.hpp>
#include <xqilla/context/ModuleResolver.hpp>
#include <xqilla/context/URIResolver.hpp>
#include <xqilla/runtime/Sequence.hpp>
/**
 * \example simple-basic.cpp
 *
 * This example executes a simple XQuery expression ("1 to 100"), which returns the numbers from 1 to 100 inclusive.
 */

/**
 * \example simple-context-item.cpp
 *
 * This example parses a document and sets it as the context item. It then executes an XQuery expression that navigates relative to the context item.
 */
