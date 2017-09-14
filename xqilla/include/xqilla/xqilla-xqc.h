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

#ifndef _XQILLA_XQC_H
#define _XQILLA_XQC_H

#include <xqc.h>

#ifdef  __cplusplus
extern "C" {
#endif

/**
 * Creates an XQC_Implementation object that uses XQilla.
 */
XQC_Implementation *createXQillaXQCImplementation(int version);

/**
 * \example xqc-basic.c
 *
 * This example executes a simple XQuery expression ("1 to 100"), which returns the numbers from 1 to 100 inclusive.
 */

/**
 * \example xqc-context-item.c
 *
 * This example parses a document and sets it as the context item. It then executes an XQuery expression that navigates relative to the context item.
 */

#ifdef  __cplusplus
}
#endif

#endif
