#!/bin/bash
# script to build on mac
build=MinSizeRel
dataCompressionLib=libcompression
bundle=off
xmlparserLib=applexml
xmlparser="-DXML_PARSER=applexml"
addressSanitizerFlag=off

usage()
{
    echo "usage: makemac [-b buildType] [-xzlib] [-parser-xerces] [-asan]"
    echo $'\t' "-b Build type. Default MinSizeRel"
    echo $'\t' "-xzlib Use MSIX SDK Zlib instead of inbox libCompression api. Default on MacOS is libCompression."
    echo $'\t' "-sb Skip bundle support."
    echo $'\t' "-parser-xerces Use xerces xml parser instead of default apple xml parser."
    echo $'\t' "-asan Turn on address sanitizer for memory corruption detection."
}

printsetup()
{
    echo "Build Type:" $build
    echo "Data Compression library:" $dataCompressionLib
    echo "Skip bundle support:" $bundle
    echo "parser:" $xmlparserLib
    echo "Address Sanitizer:" $addressSanitizerFlag
}

while [ "$1" != "" ]; do
    case $1 in
        -b )    shift
                build=$1
                ;;
        -xzlib )dataCompressionLib=MSIX_SDK_zlib
                zlib="-DUSE_MSIX_SDK_ZLIB=on"
                ;;
        -parser-xerces )  xmlparserLib=xerces
                xmlparser="-DXML_PARSER=xerces"
                ;;
        -asan ) addressSanitizerFlag=on
                addressSanitizer="-DASAN=on"
                ;;
        -sb )   bundle="on"
                ;;
        -h )    usage
                exit
                ;;
        * )     usage
                exit 1
    esac
    shift
done
printsetup

mkdir .vs
cd .vs

# clean up any old builds of msix modules
find . -depth -name *msix* | xargs -0 -r rm -rf

cmake -DCMAKE_BUILD_TYPE=$build $zlib -DSKIP_BUNDLES=$bundle $xmlparser $addressSanitizer -DMACOS=on ..
make
