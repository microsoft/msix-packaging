cmake_minimum_required( VERSION 3.8.0 )
# kudos to https://github.com/madwax/3ndparty.cmake.openssl/

include( CMakeParseArguments )
project( crypto )

set( TARGET_SOURCE_DIR_TRUE "${OpenSSL_SOURCE_PATH}/crypto" )
set( TARGET_INCLUDE_DIRS ${OpenSLL_INCLUDE_PATH} )
set( TARGET_INCLUDE_DIRS_PRIVATE "${TARGET_SOURCE_DIR_TRUE}" "${TARGET_SOURCE_DIR_TRUE}/asn1" "${TARGET_SOURCE_DIR_TRUE}/evp" "${TARGET_SOURCE_DIR_TRUE}/modes")
set( TARGET_DEFINES "OPENSSL_THREADS" )
set( TARGET_DEFINES_PRIVATE "${OpenSSL_COMPILER_DEFINES}" )
if(NOT MSVC)
    set( TARGET_COMPILE_FLAGS_PRIVATE -ffunction-sections -fdata-sections)
endif()
set( TARGET_LINK "" )
set( TARGET_LINK_PRIVATE "" )
set( TARGET_SOURCES "" )

# Because OpenSSL does silly things we have to create a proper include dir to build everything
file( COPY ${OpenSSL_SOURCE_PATH}/e_os.h DESTINATION ${OpenSLL_INCLUDE_PATH}/ )
file( COPY ${OpenSSL_SOURCE_PATH}/include/openssl/e_os2.h DESTINATION ${OpenSLL_INCLUDE_PATH}/openssl/ )

# we hold the sources (.c) under XSRC and headers (.h) under XINC
# we do this as we need to copy headers else the lib will not build.
set( XSRC "" )
set( XINC "" )
set( XSRC_SHARED "")

# OpenSSL Has a lot of source files so we separated it.
include( crypto_sources )

file( COPY ${OpenSSL_SOURCE_PATH}/include/openssl/opensslconf.h.in DESTINATION ${OpenSLL_INCLUDE_PATH}/openssl )
file( COPY ${OpenSSL_SOURCE_PATH}/include/internal DESTINATION ${OpenSLL_INCLUDE_PATH}/openssl/ )
file( COPY ${OpenSSL_SOURCE_PATH}/include/crypto DESTINATION ${OpenSLL_INCLUDE_PATH}/openssl/ )
file( COPY ${OpenSSL_SOURCE_PATH}/include/openssl DESTINATION ${OpenSLL_INCLUDE_PATH}/openssl/ )

if(WIN32)
    # TODO: Replicate build flags for cl
    # Flags taken from OpenSSL Configure file for VC-WIN64A target.  More care may be required for other targets.
    if ((CMAKE_BUILD_TYPE MATCHES Release) OR (CMAKE_BUILD_TYPE MATCHES MinSizeRel))
        set(TARGET_COMPILE_FLAGS -O1 -W3 -Gs0 -Gy -nologo -DOPENSSL_SYSNAME_WIN32 -DWIN32_LEAN_AND_MEAN -DL_ENDIAN -DUNICODE -D_UNICODE)
    else()
        set(TARGET_COMPILE_FLAGS -Zi -W3 -Gs0 -Gy -nologo -DOPENSSL_SYSNAME_WIN32 -DWIN32_LEAN_AND_MEAN -DL_ENDIAN -DUNICODE -D_UNICODE)
    endif()
else()
    set( TARGET_COMPILE_FLAGS -fno-rtti -fno-stack-protector -O1 -fno-unwind-tables -fno-asynchronous-unwind-tables
        -fno-math-errno -fno-unroll-loops -fmerge-all-constants)
endif()

if( MSVC )
  include( MSVCRuntime )
  # configure_msvc_runtime()
  set( OPENSSLDIR "C:/ssl" )
  set( ENGINESDIR "C:/engines-1.1" )
else()
  set( OPENSSLDIR "/usr/local/ssl" )
  set( ENGINESDIR "/usr/local/engines-1.1" )
endif()
add_definitions( "-DOPENSSLDIR=\"${OPENSSLDIR}\"" )
add_definitions( "-DENGINESDIR=\"${ENGINESDIR}\"" )

# This was a default option during OpenSSL 1.0.2u and is the option that MSIX SDK 
# uses although it compiles set of DSO source code. With the latest version
# of OpenSSL 1.1.1j we need this to be set explicitly (it appears we don't have default)
set( DSO_NONE ON )

if( APPLE )
  set( DSO_EXTENSION ".dylib" )
elseif( WIN32 AND NOT CYGWIN )
  set( DSO_EXTENSION ".dll" )
elseif( CYGWIN )
  set( DSO_EXTENSION ".dll" )
else()
  set( DSO_EXTENSION ".so" )
endif()

include( CheckTypeSize )
check_type_size( "long" LONG_INT )
check_type_size( "long long" LONG_LONG_INT )
check_type_size( "int" INT )
if( HAVE_LONG_INT AND (${LONG_INT} EQUAL 8) )
  set( SIXTY_FOUR_BIT_LONG ON )
elseif( HAVE_LONG_LONG_INT AND (${LONG_LONG_INT} EQUAL 8) )
  set( SIXTY_FOUR_BIT ON )
else()
  set( THIRTY_TWO_BIT ON )
endif()

# Begin configure public headers
file( READ "${MSIX_PROJECT_ROOT}/cmake/openssl/opensslconf.h.cmake" CONF )
set(CONDITIONAL_CONF "")

if(NOT MSIX_PACK)
    set(CONDITIONAL_CONF "${CONDITIONAL_CONF}
#define OPENSSL_NO_DES")
endif()
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
#define OPENSSL_NO_CAMELLIA
#define OPENSSL_NO_RIPEMD
#define OPENSSL_NO_RC5
#define OPENSSL_NO_BF
#define OPENSSL_NO_IDEA
#define OPENSSL_NO_ENGINE
#define OPENSSL_NO_MDC2
#define OPENSSL_NO_SEED
#define OPENSSL_NO_DEPRECATED
#define OPENSSL_NO_DSA
#define OPENSSL_NO_DH
#define OPENSSL_NO_EC
#define OPENSSL_NO_ECDSA
#define OPENSSL_NO_ECDH
#define OPENSSL_NO_WHIRLPOOL
#define OPENSSL_NO_RC4
#define OPENSSL_NO_RC2
#define OPENSSL_NO_SSL2
#define OPENSSL_NO_SSL3
#define OPENSSL_NO_CAST
#define OPENSSL_NO_CMAC
#define OPENSSL_NO_ZLIB
#define OPENSSL_NO_DGRAM
#define OPENSSL_NO_SOCK
#define OPENSSL_NO_BF
#define OPENSSL_NO_MD4
#define OPENSSL_NO_CMS
#define OPENSSL_NO_SRP
#define OPENSSL_NO_SM2

${CONDITIONAL_CONF}

${CONF}" )
file( WRITE "${OpenSLL_INCLUDE_PATH}/openssl/opensslconf.h.cmake" "${CONF}" )

configure_file( "${OpenSLL_INCLUDE_PATH}/openssl/opensslconf.h.cmake"
 "${OpenSLL_INCLUDE_PATH}/openssl/opensslconf.h" )

configure_file( "${MSIX_PROJECT_ROOT}/cmake/openssl/crypto/bn_conf.h.cmake"
 "${OpenSLL_INCLUDE_PATH}/openssl/crypto/bn_conf.h" )

 configure_file( "${MSIX_PROJECT_ROOT}/cmake/openssl/crypto/dso_conf.h.cmake"
 "${OpenSLL_INCLUDE_PATH}/openssl/crypto/dso_conf.h" )
# End configure public headers

set( BuildInfH " 
#ifndef MK1MF_BUILD
	/* Generated by crypto.cmake - does it break anything? */
  #define CFLAGS \"\"
  #define PLATFORM \"${CMAKE_SYSTEM_NAME}\"
  #define DATE \"\"
  static const char *compiler_flags = CFLAGS;
#endif
" )
file( WRITE ${OpenSLL_INCLUDE_PATH}/buildinf.h "${BuildInfH}" )

set(TARGET_SOURCES ${XSRC} ${XINC})

# OpenSSL is not the best when it comes to how it handles headers.  
# Where they are we need to create the projects include dir and copy stuff into it!
message(STATUS "MSIX takes a static dependency on openssl")
add_library(crypto STATIC ${TARGET_SOURCES})

target_include_directories( crypto PRIVATE ${TARGET_INCLUDE_DIRS} ${TARGET_INCLUDE_DIRS_PRIVATE} )
target_compile_definitions( crypto PRIVATE ${TARGET_DEFINES} ${TARGET_DEFINES_PRIVATE} )
target_compile_options    ( crypto PRIVATE ${TARGET_COMPILE_FLAGS} ${TARGET_COMPILE_FLAGS_PRIVATE})
target_include_directories( crypto PUBLIC  ${TARGET_INCLUDE_DIRS} ${OpenSLL_INCLUDE_PATH} ${OpenSLL_INCLUDE_PATH}/openssl)
target_compile_definitions( crypto PUBLIC  ${TARGET_DEFINES} )
target_compile_options    ( crypto PUBLIC  ${TARGET_COMPILE_FLAGS})
