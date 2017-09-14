#	$Id$
# Default path for XQilla library, and paths to
# Xerces
#
#
# XQilla libraries are named:
#   xqilla<major_version><minor_version>{d}.{lib,dll,pdb}
# e.g.
#   xqilla22.dll (release) or xqilla22d.dll (debug)
#
# Assume dependent libraries are in ../../../XXX_HOME, e.g.
#   ../../../xerces-c-src
s!@XQILLA_STATIC_LIB@!@XQILLA_LIB@s!g
s!@XQILLA_LIB@!@XQILLA_LIB_BASE@@XQILLA_VERSION_MAJOR@@XQILLA_VERSION_MINOR@!g
s!@XQILLA_LIB_BASE@!xqilla!g

s!@XQILLA_VERSION_MAJOR@!2!g
s!@XQILLA_VERSION_MINOR@!3!g
s!@XQILLA_VERSION_PATCH@!3!g

# Xerces is assumed to be source (vs installation)
s!@XERCES_LIBHOME9@!@XERCES_WINHOME@/Build!g
s!@XERCES_LIBHOME10@!@XERCES_WINHOME@/Build!g
s!@XERCES_LIBHOME11@!@XERCES_WINHOME@/Build!g
# Xerces-c is in ../../../xerces-c-src
s!@XERCES_WINHOME@!../../../@XERCES_NAME@!g
s!@XERCES_NAME@!xerces-c-src!g
s!@XERCES_STATIC_LIB@!xerces-c_static_@XERCES_VERSION_MAJOR@!g
s!@XERCES_LIB@!xerces-c_@XERCES_VERSION_MAJOR@!g
s!@XERCES_DLL@!xerces-c_@XERCES_VERSION_MAJOR@_@XERCES_VERSION_MINOR@!g

# Current Xerces version is 3.1.3
s!@XERCES_VERSION_MAJOR@!3!g
s!@XERCES_VERSION_MINOR@!1!g
s!@XERCES_VERSION_PATCH@!3!g

