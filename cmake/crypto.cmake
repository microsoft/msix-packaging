cmake_minimum_required( VERSION 3.4.0 )
# kudos to https://github.com/madwax/3ndparty.cmake.openssl/

include( CMakeParseArguments )
project( crypto )

set( TARGET_SOURCE_DIR_TRUE "${OpenSSL_SOURCE_PATH}/crypto" )
set( TARGET_INCLUDE_DIRS ${OpenSLL_INCLUDE_PATH} )
set( TARGET_INCLUDE_DIRS_PRIVATE "${TARGET_SOURCE_DIR_TRUE}" "${TARGET_SOURCE_DIR_TRUE}/asn1" "${TARGET_SOURCE_DIR_TRUE}/evp" "${TARGET_SOURCE_DIR_TRUE}/modes")
set( TARGET_DEFINES "OPENSSL_THREADS" )
set( TARGET_DEFINES_PRIVATE "${OpenSSL_COMPILER_DEFINES}" )
set( TARGET_COMPILE_FLAGS "" )
set( TARGET_COMPILE_FLAGS_PRIVATE "" )
set( TARGET_LINK "" )
set( TARGET_LINK_PRIVATE "" )
set( TARGET_SOURCES "" )

# Because OpenSSL does silly things we have to create a proper include dir to build everything
file( COPY ${OpenSSL_SOURCE_PATH}/e_os.h DESTINATION ${OpenSLL_INCLUDE_PATH} )
file( COPY ${OpenSSL_SOURCE_PATH}/e_os2.h DESTINATION ${OpenSLL_INCLUDE_PATH} )

# we hold the sources (.c) under XSRC and headers (.h) under XINC
# we do this as we need to copy headers else the lib will not build.
set( XSRC "" )
set( XINC "" )
set( XPNC "" )

# OpenSSL Has a LOT of source files so we only compile what we need
include( crypto_sources )

file( COPY ${XINC} DESTINATION ${OpenSLL_INCLUDE_PATH}/openssl FILES_MATCHING REGEX "\.h$" )
file( COPY ${XPNC} DESTINATION ${OpenSLL_INCLUDE_PATH}/internal FILES_MATCHING REGEX "\.h$" )
file( COPY ${OpenSSL_SOURCE_PATH}/crypto/opensslconf.h.in DESTINATION ${OpenSLL_INCLUDE_PATH}/openssl )

# HACK HACK HACK HACK HACK HACK HACK HACK HACK HACK HACK HACK HACK HACK HACK HACK HACK HACK HACK HACK 
# HACK   Content copied from ${OpenSLL_INCLUDE_PATH}/openssl/opensslconf.h.in and modified       HACK
# HACK   to fit our needs.  TODO: figure out how to emulate perl via CMAKE to replace tokens     HACK
# HACK HACK HACK HACK HACK HACK HACK HACK HACK HACK HACK HACK HACK HACK HACK HACK HACK HACK HACK HACK 
set( CONF "
#define OPENSSL_NO_GMP
#define OPENSSL_NO_JPAKE
#define OPENSSL_NO_KRB5
#define OPENSSL_NO_MD2
#define OPENSSL_NO_RFC3779
#define OPENSSL_NO_STORE
#define OPENSSL_NO_DYNAMIC_ENGINE
#define OPENSSL_NO_SCTP
#define OPENSSL_NO_EC_NISTP_64_GCC_128
/*
*
*
* Copyright 2016 The OpenSSL Project Authors. All Rights Reserved.
*
* Licensed under the OpenSSL license (the \"License\").  You may not use
* this file except in compliance with the License.  You can obtain a copy
* in the file LICENSE in the source distribution or at
* https://www.openssl.org/source/license.html
*/

/*
* Sometimes OPENSSSL_NO_xxx ends up with an empty file and some compilers
* don't like that.  This will hopefully silence them.
*/
#define NON_EMPTY_TRANSLATION_UNIT static void *dummy = &dummy;

/*
* Applications should use -DOPENSSL_API_COMPAT=<version> to suppress the
* declarations of functions deprecated in or before <version>. Otherwise, they
* still won't see them if the library has been built to disable deprecated
* functions.
*/
#if defined(OPENSSL_NO_DEPRECATED)
# define DECLARE_DEPRECATED(f)
#elif __GNUC__ > 3 || (__GNUC__ == 3 && __GNUC_MINOR__ > 0)
# define DECLARE_DEPRECATED(f)    f __attribute__ ((deprecated));
#else
# define DECLARE_DEPRECATED(f)   f;
#endif

#ifndef OPENSSL_FILE
# ifdef OPENSSL_NO_FILENAMES
#  define OPENSSL_FILE \"\"
#  define OPENSSL_LINE 0
# else
#  define OPENSSL_FILE __FILE__
#  define OPENSSL_LINE __LINE__
# endif
#endif

#ifndef OPENSSL_MIN_API
# define OPENSSL_MIN_API 0
#endif

#if !defined(OPENSSL_API_COMPAT) || OPENSSL_API_COMPAT < OPENSSL_MIN_API
# undef OPENSSL_API_COMPAT
# define OPENSSL_API_COMPAT OPENSSL_MIN_API
#endif

#if OPENSSL_API_COMPAT < 0x10200000L
# define DEPRECATEDIN_1_2_0(f)   DECLARE_DEPRECATED(f)
#else
# define DEPRECATEDIN_1_2_0(f)
#endif

#if OPENSSL_API_COMPAT < 0x10100000L
# define DEPRECATEDIN_1_1_0(f)   DECLARE_DEPRECATED(f)
#else
# define DEPRECATEDIN_1_1_0(f)
#endif

#if OPENSSL_API_COMPAT < 0x10000000L
# define DEPRECATEDIN_1_0_0(f)   DECLARE_DEPRECATED(f)
#else
# define DEPRECATEDIN_1_0_0(f)
#endif

#if OPENSSL_API_COMPAT < 0x00908000L
# define DEPRECATEDIN_0_9_8(f)   DECLARE_DEPRECATED(f)
#else
# define DEPRECATEDIN_0_9_8(f)
#endif

// set in crypto.cmake
#undef I386_ONLY
#undef OPENSSL_EXPORT_VAR_AS_FUNCTION
#define OPENSSL_SYS_UEFI
#define OPENSSL_SYS_UNIX
#define OPENSSL_RAND_SEED_NONE
#define BN_ULONG unsigned long
#define BN_BYTES sizeof(BN_ULONG)

typedef unsigned char       UINT8;
typedef signed char         INT8;
typedef unsigned short      UINT16;
typedef short               INT16;
typedef unsigned int        UINT32;
typedef int                 INT32;
typedef unsigned long long  UINT64;
typedef long long           INT64;

#define ossl_ssize_t        int
#define OSSL_SSIZE_MAX      INT_MAX
#define RC4_INT             char /* RC4_CHAR	use 'char' instead of 'int' for RC4_INT in crypto/rc4/rc4.h from lib/openssl/Configure */

" )
IF ((AOSP) OR (LINUX))
  set( CONF "${CONF}
  #undef OPENSSL_NO_AFALGENG
  ")
ELSE()
  set( CONF "${CONF}
  #define OPENSSL_NO_AFALGENG
  ")
ENDIF()

file( WRITE "${OpenSLL_INCLUDE_PATH}/openssl/opensslconf.h" "${CONF}" )

set( BuildInfH " 
#ifndef MK1MF_BUILD
  /* Generated byt crypto.cmake - does it break anything? */
  #define CFLAGS \"\"
  #define PLATFORM \"${CMAKE_SYSTEM_NAME}\"
  #define DATE \"\"
#endif
" )
file( WRITE ${OpenSLL_INCLUDE_PATH}/buildinf.h "${BuildInfH}" )

set( TARGET_SOURCES ${XSRC} ${XINC} )

# OpenSSL is not the best when it comes to how it handles headers.  
# Where they are we need to create the projects include dir and copy stuff into it!

add_library( crypto STATIC ${TARGET_SOURCES} )

# specify that this library is to be built with C++14
set_property(TARGET crypto PROPERTY CXX_STANDARD 14)

include_directories(
  ${include_directories}
  ${OpenSLL_INCLUDE_PATH}
)

target_include_directories( crypto PRIVATE ${TARGET_INCLUDE_DIRS} ${TARGET_INCLUDE_DIRS_PRIVATE} )
target_compile_definitions( crypto PRIVATE ${TARGET_DEFINES} ${TARGET_DEFINES_PRIVATE} )
target_link_libraries     ( crypto PRIVATE ${TARGET_LINK} ${TARGET_LINK_PRIVATE} )
target_compile_options    ( crypto PRIVATE ${TARGET_COMPILE_FLAGS} ${TARGET_COMPILE_FLAGS_PRIVATE} )
target_include_directories( crypto PUBLIC  ${TARGET_INCLUDE_DIRS} ${OpenSLL_INCLUDE_PATH}/openssl)
target_compile_definitions( crypto PUBLIC  ${TARGET_DEFINES} )
target_link_libraries     ( crypto PUBLIC  ${TARGET_LINK} )
target_compile_options    ( crypto PUBLIC  ${TARGET_COMPILE_FLAGS} )