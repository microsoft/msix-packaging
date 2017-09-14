/*
 * Copyright (c) 2008, Matthias Brantner, John Snelson
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *     * Redistributions of source code must retain the above copyright notice,
 *       this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of the developers nor the names of contributors may be
 *       used to endorse or promote products derived from this software without
 *       specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef _XQUERY_C_API_H
#define _XQUERY_C_API_H

/* Include stdio for FILE */
#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

/** The version of the XQC API in this header file */
#define XQC_VERSION_NUMBER 1

typedef struct XQC_Implementation_s XQC_Implementation;
typedef struct XQC_StaticContext_s XQC_StaticContext;
typedef struct XQC_Expression_s XQC_Expression;
typedef struct XQC_DynamicContext_s XQC_DynamicContext;
typedef struct XQC_Sequence_s XQC_Sequence;
typedef struct XQC_InputStream_s XQC_InputStream;
typedef struct XQC_ErrorHandler_s XQC_ErrorHandler;

/**
 * The error enumeration used by all XQC functions to designate error condition.
 * All XQC functions return a value of type ::XQC_Error.
 */
typedef enum {
	XQC_NO_ERROR = 0,          ///< No error.
	XQC_END_OF_SEQUENCE,       ///< The end of the XQC_Sequence has been reached.
	XQC_NO_CURRENT_ITEM,
	XQC_PARSE_ERROR,
	XQC_INVALID_ARGUMENT,
	XQC_NOT_NODE,

	XQC_INTERNAL_ERROR,        ///< An implementation specific error has occurred.
	XQC_NOT_IMPLEMENTED,       ///< The implementation does not implement that function.
	/**
	 * The encoding of the query has not been recognized, or is not supported by the
	 * implementation. All implementations must support queries in UTF-8.
	 */
	XQC_UNRECOGNIZED_ENCODING,

	XQC_STATIC_ERROR,          ///< A static error has occured while preparing the query
	XQC_TYPE_ERROR,            ///< A type error has occured while preparing or executing the query
	XQC_DYNAMIC_ERROR,         ///< A dynamic error has occured while preparing or executing the query
	XQC_SERIALIZATION_ERROR    ///< A serialization error has occured while serializing the output of a query
} XQC_Error;

/**
 * The ::XQC_InputStream struct is designed to be populated by users for the purpose
 * of streaming data into an XQC implementation.
 */
struct XQC_InputStream_s {
	/**
	 * The text encoding of the input data as a UTF-8 string, or 0 if unknown. The value of the string
	 * should conform to the <code>EncName</code> grammar production as specified in XML 1.0:
	 *
	 * http://www.w3.org/TR/REC-xml/#NT-EncName
	 */
	const char *encoding;

	/**
	 * Can be used for user specific purposes.
	 */
	void *user_data;

	/**
	 * The function called to read more of the input query. The function should read
	 * the next chunk of input into the buffer provided, returning the length of the
	 * data read.
	 *
	 * \param stream The XQC_InputStream that this function pointer is a member of
	 * \param[out] buffer The buffer to read the data into
	 * \param length The length of the buffer
	 *
	 * \return The number of bytes read - this will be less than length if the end of the input is reached
	 *
	 * \todo Does this function need to be able to return an error condition?
	 */
	unsigned int (*read)(XQC_InputStream *stream, void *buffer, unsigned int length);

	/**
	 * Called to free the resources associated with the XQC_InputStream.
	 * 
	 * \param stream The XQC_InputStream that this function pointer is a member of
	 *
	 */
	void (*free)(XQC_InputStream *stream);
};

/**
 * The ::XQC_ErrorHandler struct is designed to be populated by users for the purpose
 * of collecting more detailed error messages from an XQC implementation. An XQC_ErrorHandler
 * can be set for a query execution using the XQC_StaticContext::set_error_handler() and
 * XQC_DynamicContext::set_error_handler() functions.
 *
 * The XQC_ErrorHandler
 * struct has no free() function pointer because the user remains responsible for freeing
 * the resources associated with this struct.
 *
 * \todo file/line/column information?
 */
struct XQC_ErrorHandler_s {

	/**
	 * Can be used for user specific purposes.
	 */
	void *user_data;

	/**
	 * The function called when an error occurs. The function receives the components of the
	 * error as arguments. When this function returns, the implementation will exit query preparation or
	 * execution with the error enumeration value passed as an argument.
	 *
	 * \param handler The XQC_ErrorHandler that this function pointer is a member of
	 * \param error An enumeration value representing the type of error. One of either XQC_STATIC_ERROR,
	 * XQC_TYPE_ERROR, XQC_DYNAMIC_ERROR, or XQC_SERIALIZATION_ERROR.
	 * \param error_uri The namespace URI of the error code QName as a UTF-8 string, or 0 if there
	 * is no namespace URI.
	 * \param error_localname The local name of the error code QName as a UTF-8 string.
	 * \param description The description of the error message as a UTF-8 string. The description may be
	 * absent, in which case this parameter will be 0.
	 * \param error_object The error object, potentially passed to the <code>fn:error()</code> function.
	 * The user owns this object, and is responsible for freeing it. The error_object may be absent, in
	 * which case this parameter will be 0. Some implementations may not provide this functionality,
	 * meaning that this parameter will always be 0.
	 */
	void (*error)(XQC_ErrorHandler *handler, XQC_Error error, const char *error_uri,
		const char *error_localname, const char *description, XQC_Sequence *error_object);
};

typedef enum {
	XQC_EMPTY_TYPE = 0,

	XQC_DOCUMENT_TYPE,
	XQC_ELEMENT_TYPE,
	XQC_ATTRIBUTE_TYPE,
	XQC_TEXT_TYPE,
	XQC_PROCESSING_INSTRUCTION_TYPE,
	XQC_COMMENT_TYPE,
	XQC_NAMESPACE_TYPE,

	XQC_ANY_SIMPLE_TYPE,
	XQC_ANY_URI_TYPE,
	XQC_BASE_64_BINARY_TYPE,
	XQC_BOOLEAN_TYPE,
	XQC_DATE_TYPE,
	XQC_DATE_TIME_TYPE,
	XQC_DAY_TIME_DURATION_TYPE,
	XQC_DECIMAL_TYPE,
	XQC_DOUBLE_TYPE,
	XQC_DURATION_TYPE,
	XQC_FLOAT_TYPE,
	XQC_G_DAY_TYPE,
	XQC_G_MONTH_TYPE,
	XQC_G_MONTH_DAY_TYPE,
	XQC_G_YEAR_TYPE,
	XQC_G_YEAR_MONTH_TYPE,
	XQC_HEX_BINARY_TYPE,
	XQC_NOTATION_TYPE,
	XQC_QNAME_TYPE,
	XQC_STRING_TYPE,
	XQC_TIME_TYPE,
	XQC_UNTYPED_ATOMIC_TYPE,
	XQC_YEAR_MONTH_DURATION_TYPE
	
} XQC_ItemType;

/**
 * The ::XQC_Implementation struct provides factory functions for preparing queries. An XQC_Implementation object
 * is thread-safe and can be used by multiple threads of execution at the same time.
 *
 * The method of creating an
 * XQC_Implementation object is beyond the scope of this API, and will typically involve calling an
 * implementation defined function. Once created, the user is responsible for freeing the object by calling
 * the free() function. The XQC_Implementation object should not be freed before all objects created using it's
 * functions have been freed - doing so may cause undefined behaviour.
 */
struct XQC_Implementation_s {

	/**
	 * \name Functions for preparing queries
	 * @{
	 */

	/**
	 * Creates a static context suitable for use in the prepare(), prepare_file() and prepare_stream()
	 * functions. The user is responsible for freeing the ::XQC_StaticContext object returned by calling
	 * XQC_StaticContext::free().
	 *
	 * \param implementation The XQC_Implementation that this function pointer is a member of
	 * \param[out] context The newly created XQC_StaticContext object.
	 *
	 * \retval ::XQC_NO_ERROR
	 * \retval ::XQC_INTERNAL_ERROR
	 */
	XQC_Error (*create_context)(XQC_Implementation *implementation, XQC_StaticContext **context);

	/**
	 * Prepares a query from a UTF-8 string, returning an ::XQC_Expression object. The user remains responsible
	 * for closing the file after preparation. The user is responsible for freeing the ::XQC_Expression object
	 * returned by calling XQC_Expression::free().
	 *
	 * \param implementation The XQC_Implementation that this function pointer is a member of.
	 * \param string The query to prepare as a UTF-8 string.
	 * \param context The initial static context for this query, or 0 to use the implementation defined
	 * default static context.
	 * \param[out] expression The resulting prepared expression.
	 *
	 * \retval ::XQC_NO_ERROR
	 * \retval ::XQC_INTERNAL_ERROR
	 * \retval ::XQC_STATIC_ERROR
	 * \retval ::XQC_TYPE_ERROR
	 * \retval ::XQC_DYNAMIC_ERROR
	 */
	XQC_Error (*prepare)(XQC_Implementation *implementation, const char *string,
		const XQC_StaticContext *context, XQC_Expression **expression);

	/**
	 * Prepares a query from a FILE pointer, returning an ::XQC_Expression object. The encoding of the
	 * query in the file is determined by the implementation. The user remains responsible for closing
	 * the file after preparation. The user is responsible for freeing the ::XQC_Expression object returned by
	 * calling XQC_Expression::free().
	 *
	 * \param implementation The XQC_Implementation that this function pointer is a member of.
	 * \param file The file containing the query to prepare.
	 * \param context The initial static context for this query, or 0 to use the implementation defined
	 * default static context.
	 * \param[out] expression The resulting prepared expression.
	 *
	 * \retval ::XQC_NO_ERROR
	 * \retval ::XQC_INTERNAL_ERROR
	 * \retval ::XQC_UNRECOGNIZED_ENCODING
	 * \retval ::XQC_STATIC_ERROR
	 * \retval ::XQC_TYPE_ERROR
	 * \retval ::XQC_DYNAMIC_ERROR
	 */
	XQC_Error (*prepare_file)(XQC_Implementation *implementation, FILE *file,
		const XQC_StaticContext *context, XQC_Expression **expression);

	/**
	 * Prepares a query from an ::XQC_InputStream, returning an ::XQC_Expression object. The encoding of the stream
	 * is determined by looking at XQC_InputStream::encoding, or by the implementation if
	 * XQC_InputStream::encoding is 0.
	 * The implementation is responsible for freeing the ::XQC_InputStream using the XQC_InputStream::free()
	 * function after it has finished with using it. The user is responsible for freeing the ::XQC_Expression
	 * object returned by calling XQC_Expression::free().
	 *
	 * \param implementation The XQC_Implementation that this function pointer is a member of
	 * \param stream The stream returning the query to prepare.
	 * \param context The initial static context for this query, or 0 to use the implementation defined
	 * default static context.
	 * \param[out] expression The resulting prepared expression
	 *
	 * \retval ::XQC_NO_ERROR
	 * \retval ::XQC_INTERNAL_ERROR
	 * \retval ::XQC_UNRECOGNIZED_ENCODING
	 * \retval ::XQC_STATIC_ERROR
	 * \retval ::XQC_TYPE_ERROR
	 * \retval ::XQC_DYNAMIC_ERROR
	 */
	XQC_Error (*prepare_stream)(XQC_Implementation *implementation, XQC_InputStream *stream,
		const XQC_StaticContext *context, XQC_Expression **expression);

	/// @}

	/**
	 * \name Functions for parsing documents
	 * @{
	 */

	/// XQC_PARSE_ERROR
	XQC_Error (*parse_document)(XQC_Implementation *implementation,
		const char *string, XQC_Sequence **sequence);
	/// XQC_PARSE_ERROR
	XQC_Error (*parse_document_file)(XQC_Implementation *implementation,
		FILE *file, XQC_Sequence **sequence);
	/// XQC_PARSE_ERROR
	XQC_Error (*parse_document_stream)(XQC_Implementation *implementation,
		XQC_InputStream *stream, XQC_Sequence **sequence);

	/// @}

	/**
	 * \name Functions for creating sequences
	 * @{
	 */

	XQC_Error (*create_empty_sequence)(XQC_Implementation *implementation,
		XQC_Sequence **sequence);
	XQC_Error (*create_singleton_sequence)(XQC_Implementation *implementation,
		XQC_ItemType type, const char *value,
		XQC_Sequence **sequence);
	XQC_Error (*create_string_sequence)(XQC_Implementation *implementation,
		const char *values[], unsigned int count,
		XQC_Sequence **sequence);
	XQC_Error (*create_integer_sequence)(XQC_Implementation *implementation,
		int values[], unsigned int count,
		XQC_Sequence **sequence);
	XQC_Error (*create_double_sequence)(XQC_Implementation *implementation,
		double values[], unsigned int count,
		XQC_Sequence **sequence);

	/// @}

	/**
	 * Called to retrieve an implementation specific interface.
	 * 
	 * \param implementation The XQC_Implementation that this function pointer is a member of
	 * \param name The name that identifies the interface to return
	 *
	 * \return A pointer to the interface, or 0 if the name is not recognized by this
	 * implementation of XQC.
	 */
	void *(*get_interface)(const XQC_Implementation *implementation, const char *name);

	/**
	 * Called to free the resources associated with the XQC_Implementation.
	 * 
	 * \param implementation The XQC_Implementation that this function pointer is a member of
	 *
	 */
	void (*free)(XQC_Implementation *implementation);
};

/** 
 * XPath 1.0 compatibility mode as defined in
 * http://www.w3.org/TR/xquery/#static_context
 * \todo Are there better enumeration names?
 */
typedef enum { XQC_XPATH2_0, XQC_XPATH1_0 } XQC_XPath1Mode;

/** 
 * Ordering mode as defined in http://www.w3.org/TR/xquery/#static_context.
 */
typedef enum { XQC_ORDERED, XQC_UNORDERED } XQC_OrderingMode;

/** 
 * Default order for empty sequences as defined in
 * http://www.w3.org/TR/xquery/#static_context.
 */
typedef enum { XQC_EMPTY_GREATEST, XQC_EMPTY_LEAST } XQC_OrderEmptyMode;

/**
 * Inherit part of the Copy-namespace mode as defined in
 * http://www.w3.org/TR/xquery/#static_context.
 */
typedef enum { XQC_INHERIT_NS, XQC_NO_INHERIT_NS } XQC_InheritMode;

/** 
 * Preserve part of the Copy-namespace mode as defined in
 * http://www.w3.org/TR/xquery/#static_context.
 */
typedef enum { XQC_PRESERVE_NS, XQC_NO_PRESERVE_NS } XQC_PreserveMode;

/** 
 * Boundary-space policy as defined in http://www.w3.org/TR/xquery/#static_context.
 */
typedef enum { XQC_PRESERVE_SPACE, XQC_STRIP_SPACE } XQC_BoundarySpaceMode;

/** 
 * Construction mode as defined in http://www.w3.org/TR/xquery/#static_context.
 */
typedef enum { XQC_PRESERVE_CONS, XQC_STRIP_CONS } XQC_ConstructionMode;

/**
 * The ::XQC_StaticContext struct provides a way to specify values for the static context of the query to be
 * prepared. An ::XQC_StaticContext object is not thread-safe - threads should each use their own instance of a
 * ::XQC_StaticContext object.
 *
 * ::XQC_StaticContext objects are created by calling the XQC_Implementation::create_context() function. Once
 * created, the user is responsible for freeing the object by calling
 * the free() function. The ::XQC_StaticContext object should be freed before the ::XQC_Implementation object that
 * created it.
 */
struct XQC_StaticContext_s {

	/**
	 * Creates a child context of the given static context.
	 * A child context contains the same information as it's parent context but
	 * it allows the user to override and add information.
	 * The user is responsible for freeing the ::XQC_StaticContext object returned by calling
	 * XQC_StaticContext::free().
	 *
	 * \param context The XQC_StaticContext that this function pointer is a member of
	 * \param[out] child_context The newly created XQC_StaticContext object which is
	 *             a child of the given context.
	 *
	 * \retval ::XQC_NO_ERROR
	 * \retval ::XQC_INTERNAL_ERROR
	 */
	XQC_Error
	(*create_child_context)(XQC_StaticContext *context, XQC_StaticContext **child_context);

	/**
	 * Adds a (prefix, uri) pair to the set of statically known namespaces of
	 * the given context.
	 *
	 * \param context The XQC_StaticContext that this function pointer is a member of
	 * \param prefix The prefix of the namespace to add to the given XQC_StaticContext.
	 * \param uri    The uri of the namespace to add to the given XQC_StaticContext.
	 *
	 * \retval ::XQC_NO_ERROR
	 * \retval ::XQC_INTERNAL_ERROR
	 */
	XQC_Error
	(*declare_ns)(XQC_StaticContext *context, const char *prefix, const char *uri);

	/**
	 * Returns the namespace uri that belongs to the given prefix.
	 *
	 * \param context The XQC_StaticContext that this function pointer is a member of
	 * \param prefix The prefix of the namespace to add to the given XQC_StaticContext.
	 * \param[out] result_ns The namespace uri of the namespace registered with the given prefix,
	 *   or 0 if none can be found.
	 *
	 * \retval ::XQC_NO_ERROR
	 * \retval ::XQC_INTERNAL_ERROR
	 */
	XQC_Error
	(*get_ns_by_prefix)(XQC_StaticContext *context, const char *prefix, const char **result_ns);

	/**
	 * Sets the value of the default namespace for elements and types.
	 *
	 * \param context The XQC_StaticContext that this function pointer is a member of
	 * \param uri The uri of the default element and type namespace to set in the given context.
	 *
	 * \retval ::XQC_NO_ERROR
	 * \retval ::XQC_INTERNAL_ERROR
	 */
	XQC_Error
	(*set_default_element_and_type_ns)(XQC_StaticContext *context, const char *uri);

	/**
	 *
	 * \param context The XQC_StaticContext that this function pointer is a member of
	 * \param[out] uri The uri of the default element and type namespace that is set in the given context.
	 *
	 * \retval ::XQC_NO_ERROR
	 * \retval ::XQC_INTERNAL_ERROR
	 */
	XQC_Error
	(*get_default_element_and_type_ns)(XQC_StaticContext *context, const char **uri);

	/**
	 * Sets the default namespace for functions.
	 *
	 * \param context The XQC_StaticContext that this function pointer is a member of
	 * \param uri The uri of the default function namespace to set in the given context.
	 *
	 * \retval ::XQC_NO_ERROR
	 * \retval ::XQC_INTERNAL_ERROR
	 */
	XQC_Error
	(*set_default_function_ns)(XQC_StaticContext *context, const char *uri);

	/**
	 * Returnsthe default namespace for functions set in this static context.
	 *
	 * \param context The XQC_StaticContext that this function pointer is a member of
	 * \param[out] uri The uri of the default function namespace that is set in the given context.
	 *
	 * \retval ::XQC_NO_ERROR
	 * \retval ::XQC_INTERNAL_ERROR
	 */
	XQC_Error
	(*get_default_function_ns)(XQC_StaticContext *context, const char **uri);

	/**
	 * Sets the XPath 1.0 compatibility mode to either XQC_XPATH1_0 or XQC_XPATH2_0.
	 *
	 * \param context The XQC_StaticContext that this function pointer is a member of
	 * \param mode The XQC_XPath1Mode to set in the given context.
	 *
	 * \retval ::XQC_NO_ERROR
	 * \retval ::XQC_INTERNAL_ERROR
	 */
	XQC_Error
	(*set_xpath_compatib_mode)(XQC_StaticContext *context, XQC_XPath1Mode mode);

	/**
	 * Returns the XPath 1.0 compatibility that is set in the given static context.
	 *
	 * \param context The XQC_StaticContext that this function pointer is a member of
	 * \param[out] mode The XQC_XPath1Mode that is set in the given context.
	 *
	 * \retval ::XQC_NO_ERROR
	 * \retval ::XQC_INTERNAL_ERROR
	 */
	XQC_Error 
	(*get_xpath_compatib_mode)(XQC_StaticContext *context, XQC_XPath1Mode* mode);

	/**
	 * Sets the construction mode to either XQC_PRESERVE_CONS or XQC_StaticContext.
	 *
	 * \param context The XQC_StaticContext that this function pointer is a member of
	 * \param mode The XQC_ConstructionMode to set in the given context.
	 *
	 * \retval ::XQC_NO_ERROR
	 * \retval ::XQC_INTERNAL_ERROR
	 */
	XQC_Error
	(*set_construction_mode)(XQC_StaticContext *context, XQC_ConstructionMode mode);

	/**
	 * Returns the construction mode that is set in the given static context.
	 *
	 * \param context The XQC_StaticContext that this function pointer is a member of
	 * \param[out] mode The XQC_ConstructionMode that is set in the given context.
	 *
	 * \retval ::XQC_NO_ERROR
	 * \retval ::XQC_INTERNAL_ERROR
	 */
	XQC_Error
	(*get_construction_mode)(XQC_StaticContext *context, XQC_ConstructionMode* mode);

	/**
	 * Sets the ordering mode to either XQC_ORDERED or XQC_UNORDERED.
	 *
	 * \param context The XQC_StaticContext that this function pointer is a member of
	 * \param mode The XQC_OrderingMode to set in the given context.
	 *
	 * \retval ::XQC_NO_ERROR
	 * \retval ::XQC_INTERNAL_ERROR
	 */
	XQC_Error
	(*set_ordering_mode)(XQC_StaticContext *context, XQC_OrderingMode mode);

	/**
	 * Returns the ordering mode that is set in the given static context.
	 *
	 * \param context The XQC_StaticContext that this function pointer is a member of
	 * \param[out] mode The XQC_OrderingMode that is set in the given context.
	 *
	 * \retval ::XQC_NO_ERROR
	 * \retval ::XQC_INTERNAL_ERROR
	 */
	XQC_Error
	(*get_ordering_mode)(XQC_StaticContext *context, XQC_OrderingMode* mode);

	/**
	 * Sets the default order mode for empty sequences to either XQC_EMTPY_LEAST or
	 * XQC_EMPTY_GREATEST
	 *
	 * \param context The XQC_StaticContext that this function pointer is a member of
	 * \param mode The XQC_OrderEmptyMode to set in the given context.
	 *
	 * \retval ::XQC_NO_ERROR
	 * \retval ::XQC_INTERNAL_ERROR
	 */
	XQC_Error
	(*set_default_order_empty_sequences)(XQC_StaticContext *context, XQC_OrderEmptyMode mode);

	/**
	 * Returns the default order mode for empty sequences that is set in the given
	 * static context.
	 *
	 * \param context The XQC_StaticContext that this function pointer is a member of
	 * \param[out] mode The XQC_OrderEmptyMode that is set in the given context.
	 *
	 * \retval ::XQC_NO_ERROR
	 * \retval ::XQC_INTERNAL_ERROR
	 */
	XQC_Error
	(*get_default_order_empty_sequences)(XQC_StaticContext *context, XQC_OrderEmptyMode* mode);

	/**
	 * Sets the boundary space policy to either XQC_PRESERVE_SPACE or XQC_STRIP_SPACE.
	 *
	 * \param context The XQC_StaticContext that this function pointer is a member of
	 * \param mode The XQC_BoundarySpaceMode to set in the given context.
	 *
	 * \retval ::XQC_NO_ERROR
	 * \retval ::XQC_INTERNAL_ERROR
	 */
	XQC_Error  
	(*set_boundary_space_policy)(XQC_StaticContext *context, XQC_BoundarySpaceMode mode);

	/**
	 * Returns the boundary space policy that is set in the given static context.
	 *
	 * \param context The XQC_StaticContext that this function pointer is a member of
	 * \param[out] mode The XQC_BoundarySpaceMode that is set in the given context.
	 *
	 * \retval ::XQC_NO_ERROR
	 * \retval ::XQC_INTERNAL_ERROR
	 */
	XQC_Error
	(*get_boundary_space_policy)(XQC_StaticContext *context, XQC_BoundarySpaceMode* mode);

	/**
	 * Sets the copy namespace mode which consists of the preserve and the inherit mode.
	 *
	 * \param context The XQC_StaticContext that this function pointer is a member of
	 * \param preserve The XQC_PreserveMode to set in the given context.
	 * \param inherit The XQC_InheritMode to set in the given context.
	 *
	 * \retval ::XQC_NO_ERROR
	 * \retval ::XQC_INTERNAL_ERROR
	 */
	XQC_Error 
	(*set_copy_ns_mode)(XQC_StaticContext *context, XQC_PreserveMode preserve, XQC_InheritMode inherit);

	/**
	 * Returns the copy namespace mode as a pair consisting of the preserve and the inherit
	 * mode.
	 *
	 * \param context The XQC_StaticContext that this function pointer is a member of
	 * \param[out] preserve The XQC_PreserveMode that is set in the given context.
	 * \param[out] inherit The XQC_InheritMode that is set in the given context.
	 *
	 * \retval ::XQC_NO_ERROR
	 * \retval ::XQC_INTERNAL_ERROR
	 */
	XQC_Error
	(*get_copy_ns_mode)(XQC_StaticContext *context, XQC_PreserveMode* preserve, XQC_InheritMode* inherit);

	/**
	 * Sets the base uri in the given static context.
	 *
	 * \param context The XQC_StaticContext that this function pointer is a member of
	 * \param base_uri The base uri to set in the given context.
	 *
	 * \retval ::XQC_NO_ERROR
	 * \retval ::XQC_INTERNAL_ERROR
	 */
	XQC_Error
	(*set_base_uri)(XQC_StaticContext *context, const char *base_uri);

	/**
	 * Returns the base uri that is set in the given static context.
	 *
	 * \param context The XQC_StaticContext that this function pointer is a member of
	 * \param[out] base_uri The base uri that is set in the given context.
	 *
	 * \retval ::XQC_NO_ERROR
	 * \retval ::XQC_INTERNAL_ERROR
	 */
	XQC_Error
	(*get_base_uri)(XQC_StaticContext *context, const char **base_uri);

	XQC_Error (*set_error_handler)(XQC_StaticContext *context, XQC_ErrorHandler *handler);
	XQC_Error (*get_error_handler)(const XQC_StaticContext *context, XQC_ErrorHandler **handler);

	/**
	 * Called to retrieve an implementation specific interface.
	 * 
	 * \param context The XQC_StaticContext that this function pointer is a member of
	 * \param name The name that identifies the interface to return
	 *
	 * \return A pointer to the interface, or 0 if the name is not recognized by this
	 * implementation of XQC.
	 */
	void *(*get_interface)(const XQC_StaticContext *context, const char *name);

	/**
	 * Called to free the resources associated with the XQC_StaticContext.
	 * 
	 * \param context The XQC_StaticContext that this function pointer is a member of
	 *
	 */
	void (*free)(XQC_StaticContext *context);
};

/**
 * The ::XQC_Expression struct represents a prepared query, and allows the user to execute that query any
 * number of times. An ::XQC_Expression object is thread-safe and can be used by multiple threads of execution
 * at the same time.
 *
 * ::XQC_Expression objects are created by calling the XQC_Implementation::prepare(), XQC_Implementation::prepare_file()
 * and XQC_Implementation::prepare_stream() functions. Once created, the user is responsible for freeing the object
 * by calling the free() function. The ::XQC_Expression object should be freed before the ::XQC_Implementation object
 * that created it.
 *
 * \todo A way to serialize the query result
 * \todo event api?
 */
struct XQC_Expression_s {

	/**
	 * Creates a dynamic context suitable for use in the execute() function.
	 * The user is responsible for freeing the ::XQC_DynamicContext object returned by calling
	 * XQC_DynamicContext::free().
	 *
	 * \param expression The XQC_Expression that this function pointer is a member of.
	 * \param[out] context The newly created XQC_DynamicContext object.
	 *
	 * \retval ::XQC_NO_ERROR
	 * \retval ::XQC_INTERNAL_ERROR
	 */
	XQC_Error (*create_context)(const XQC_Expression *expression, XQC_DynamicContext **context);

	/**
	 * Executes the query represented by the XQC_Expression object using the values in the
	 * ::XQC_DynamicContext if provided. An ::XQC_Sequence object is returned which can be used to
	 * examine the results of the query execution. The user is responsible for freeing the
	 * ::XQC_Sequence object returned by calling XQC_Sequence::free().
	 *
	 * \param expression The XQC_Expression that this function pointer is a member of.
	 * \param context The dynamic context information to use when executing the query, or 0 to
	 * use the implementation defined default dynamic context.
	 * \param[out] sequence The newly created XQC_Sequence object.
	 *
	 * \retval ::XQC_NO_ERROR
	 * \retval ::XQC_INTERNAL_ERROR
	 * \retval ::XQC_TYPE_ERROR
	 * \retval ::XQC_DYNAMIC_ERROR
	 */
	XQC_Error (*execute)(const XQC_Expression *expression, const XQC_DynamicContext *context, XQC_Sequence **sequence);

	/**
	 * Called to retrieve an implementation specific interface.
	 * 
	 * \param expression The XQC_Expression that this function pointer is a member of.
	 * \param name The name that identifies the interface to return
	 *
	 * \return A pointer to the interface, or 0 if the name is not recognized by this
	 * implementation of XQC.
	 */
	void *(*get_interface)(const XQC_Expression *expression, const char *name);

	/**
	 * Called to free the resources associated with the XQC_Expression.
	 * 
	 * \param expression The XQC_Expression that this function pointer is a member of
	 *
	 */
	void (*free)(XQC_Expression *expression);
};

struct XQC_DynamicContext_s {
	/**
	 * Sets the external variable to the value given. The implementation takes ownership
	 * of the XQC_Sequence passed in, and is responsible for freeing it.
	 *
	 * \param context The XQC_DynamicContext that this function pointer is a member of
	 * \param uri The namespace URI of the external variable to set.
	 * \param name The name of the external variable to set - this should be a valid lexical <code>xs:QName</code>.
	 * If <code>uri</code> is 0 and <code>name</code> has a prefix, that prefix is resolved using the in-scope
	 * namespace prefixes for the expression.
	 * \param value The XQC_Sequence value for the variable, or 0 to remove the existing
	 * binding for the variable.
	 *
	 * \retval ::XQC_NO_ERROR
	 * \retval ::XQC_INTERNAL_ERROR
	 *
	 * \todo What happens if the variable value is the wrong type?
	 * \todo Do we allow the value to come from another implementation?
	 */
	XQC_Error (*set_variable)(XQC_DynamicContext *context, const char *uri, const char *name,
		XQC_Sequence *value);

	XQC_Error (*get_variable)(const XQC_DynamicContext *context, const char *uri, const char *name,
		XQC_Sequence **value);

	/**
	 * Sets the context item to the current item of the XQC_Sequence given. The user remains
	 * responsible for freeing the XQC_Sequence passed as the value - the XQC_Sequence must
	 * not be freed until the XQC_DynamicContext has been freed or it's context item set to
	 * a different value.
	 *
	 * \param context The XQC_DynamicContext that this function pointer is a member of
	 * \param value The XQC_Sequence value for the context item, or 0 to remove the existing
	 * context item value.
	 *
	 * \retval ::XQC_NO_ERROR
	 * \retval ::XQC_INTERNAL_ERROR
	 * \retval ::XQC_NO_CURRENT_ITEM if there is no current item in the value.
	 *
	 * \todo What happens if the context item value is the wrong type?
	 * \todo Do we allow the value to come from another implementation?
	 */
	XQC_Error (*set_context_item)(XQC_DynamicContext *context, XQC_Sequence *value);

	XQC_Error (*get_context_item)(const XQC_DynamicContext *context, XQC_Sequence **value);

	/**
	 * The timezone given must be between -840 and +840 minutes (-14 and +14 hours).
	 *
	 * \param timezone The implicit timezone to set, as an offset in minutes from GMT
	 */
	XQC_Error (*set_implicit_timezone)(XQC_DynamicContext *context, int timezone);
	XQC_Error (*get_implicit_timezone)(const XQC_DynamicContext *context, int *timezone);

	XQC_Error (*set_error_handler)(XQC_DynamicContext *context, XQC_ErrorHandler *handler);
	XQC_Error (*get_error_handler)(const XQC_DynamicContext *context, XQC_ErrorHandler **handler);

	/**
	 * Called to retrieve an implementation specific interface.
	 * 
	 * \param context The XQC_DynamicContext that this function pointer is a member of
	 * \param name The name that identifies the interface to return
	 *
	 * \return A pointer to the interface, or 0 if the name is not recognized by this
	 * implementation of XQC.
	 */
	void *(*get_interface)(const XQC_DynamicContext *context, const char *name);

	/**
	 * Called to free the resources associated with the XQC_DynamicContext.
	 * 
	 * \param context The XQC_DynamicContext that this function pointer is a member of
	 *
	 */
	void (*free)(XQC_DynamicContext *context);
};

/**
 * \todo other data model node accessors (typed value, parent, attributes, children)?
 * \todo accessor for the parts of an xs:QName ?
 * \todo serialize a node
 * \todo a way to concatenate two sequences?
 */
struct XQC_Sequence_s {
	/**
	 * Moves the XQC_Sequence so that the current item is positioned at the next item in the sequence.
	 *
	 * \param sequence The XQC_Sequence that this function pointer is a member of
	 *
	 * \retval ::XQC_NO_ERROR when the call is successful
	 * \retval ::XQC_END_OF_SEQUENCE when the end of the sequence is reached
	 * \retval ::XQC_TYPE_ERROR
	 * \retval ::XQC_DYNAMIC_ERROR
	 */
	XQC_Error (*next)(XQC_Sequence *sequence);

	/**
	 * \name Functions on the current item
	 * @{
	 */

	/**
	 * Returns an item type enumeration for the type of the current item.
	 *
	 * \param sequence The XQC_Sequence that this function pointer is a member of
	 * \param[out] type the XQC_ItemType of the current item
	 *
	 * \retval ::XQC_NO_ERROR
	 * \retval ::XQC_NO_CURRENT_ITEM if there is no current item, either because next()
	 *   has not been called yet, or because the end of the sequence has been reached.
	 */
	XQC_Error (*item_type)(const XQC_Sequence *sequence, XQC_ItemType *type);

	/**
	 * Returns the type name for the current item as a (URI, localname) pair.
	 *
	 * \param sequence The XQC_Sequence that this function pointer is a member of
	 * \param[out] uri The URI of the type of the current item. The memory for the string will be valid
	 * until a subsequent call to XQC_Sequence::next().
	 * \param[out] name The localname of the type of the current item. The memory for the string will be valid
	 * until a subsequent call to XQC_Sequence::next().
	 *
	 * \retval ::XQC_NO_ERROR
	 * \retval ::XQC_NO_CURRENT_ITEM if there is no current item, either because next()
	 *   has not been called yet, or because the end of the sequence has been reached.
	 */
	XQC_Error (*type_name)(const XQC_Sequence *sequence, const char **uri, const char **name);

	/**
	 * Returns the string value of the current item in the sequence - this is equivalent to calling
	 * <code>fn:string()</code> (http://www.w3.org/TR/xpath-functions/#func-string) on the current item.
	 * This is available for all item types.
	 *
	 * \param sequence The XQC_Sequence that this function pointer is a member of
	 * \param[out] value The string value of the current item. The memory for the string will be valid
	 * until a subsequent call to XQC_Sequence::next().
	 *
	 * \retval ::XQC_NO_ERROR
	 * \retval ::XQC_NO_CURRENT_ITEM if there is no current item, either because next()
	 *   has not been called yet, or because the end of the sequence has been reached.
	 */
	XQC_Error (*string_value)(const XQC_Sequence *sequence, const char **value);

	/**
	 * Returns the value of the current item in the sequence as an integer - this is equivalent to calling
	 * <code>fn:number()</code> (http://www.w3.org/TR/xpath-functions/#func-number) on the current item, and
	 * casting the result to an int. This is available for all item types.
	 *
	 * \param sequence The XQC_Sequence that this function pointer is a member of
	 * \param[out] value The value of the current item as an int.
	 *
	 * \retval ::XQC_NO_ERROR
	 * \retval ::XQC_NO_CURRENT_ITEM if there is no current item, either because next()
	 *   has not been called yet, or because the end of the sequence has been reached.
	 */
	XQC_Error (*integer_value)(const XQC_Sequence *sequence, int *value);

	/**
	 * Returns the value of the current item in the sequence as a double - this is equivalent to calling
	 * <code>fn:number()</code> (http://www.w3.org/TR/xpath-functions/#func-number) on the current item.
	 * This is available for all item types.
	 *
	 * \param sequence The XQC_Sequence that this function pointer is a member of
	 * \param[out] value The value of the current item as a double.
	 *
	 * \retval ::XQC_NO_ERROR
	 * \retval ::XQC_NO_CURRENT_ITEM if there is no current item, either because next()
	 *   has not been called yet, or because the end of the sequence has been reached.
	 */
	XQC_Error (*double_value)(const XQC_Sequence *sequence, double *value);

	/**
	 * Returns the name for the current node as a (URI, localname) pair.
	 *
	 * \param sequence The XQC_Sequence that this function pointer is a member of
	 * \param[out] uri The URI of the name of the current node. The memory for the string will be valid
	 * until a subsequent call to XQC_Sequence::next().
	 * \param[out] name The localname of the name of the current node. The memory for the string will be valid
	 * until a subsequent call to XQC_Sequence::next().
	 *
	 * \retval ::XQC_NO_ERROR
	 * \retval ::XQC_NO_CURRENT_ITEM if there is no current item, either because next()
	 *   has not been called yet, or because the end of the sequence has been reached.
	 * \retval ::XQC_NOT_NODE if the current item is not a node.
	 */
	XQC_Error (*node_name)(const XQC_Sequence *sequence, const char **uri, const char **name);

	/** @} */

	/**
	 * Called to retrieve an implementation specific interface.
	 * 
	 * \param sequence The XQC_Sequence that this function pointer is a member of
	 * \param name The name that identifies the interface to return
	 *
	 * \return A pointer to the interface, or 0 if the name is not recognized by this
	 * implementation of XQC.
	 */
	void *(*get_interface)(const XQC_Sequence *sequence, const char *name);

	/**
	 * Called to free the resources associated with the XQC_Sequence.
	 * 
	 * \param sequence The XQC_Sequence that this function pointer is a member of
	 *
	 */
	void (*free)(XQC_Sequence *sequence);
};

#ifdef  __cplusplus
}
#endif

#endif
