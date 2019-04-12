#!/bin/bash
# script to build on mac
build=MinSizeRel
dataCompressionLib=libcompression
bundle=off
xmlparserLib=applexml
xmlparser="-DXML_PARSER=applexml"
addressSanitizerFlag=off
validationParser=off

usage()
{
    echo "usage: makemac [-b buildType] [-xzlib] [-parser-xerces] [-asan]"
    echo $'\t' "-b Build type. Default MinSizeRel"
    echo $'\t' "-xzlib Use MSIX SDK Zlib instead of inbox libCompression api. Default on MacOS is libCompression."
    echo $'\t' "-sb Skip bundle support."
    echo $'\t' "-parser-xerces Use xerces xml parser instead of default apple xml parser."
    echo $'\t' "-asan Turn on address sanitizer for memory corruption detection."
    echo $'\t' "--validation-parser, -vp Enable XML schema validation."
}

printsetup()
{
    echo "Build Type:" $build
    echo "Data Compression library:" $dataCompressionLib
    echo "Skip bundle support:" $bundle
    echo "parser:" $xmlparserLib
    echo "Address Sanitizer:" $addressSanitizerFlag
    echo "Validation parser:" $validationParser
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
        --validation-parser ) validationParser=on
                ;;
        -vp )   validationParser=on
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
find . -name *msix* -d | xargs rm -r

echo "cmake -DCMAKE_BUILD_TYPE="$build $zlib "-DSKIP_BUNDLES="$bundle $xmlparser $addressSanitizer "-DUSE_VALIDATION_PARSER="$validationParser "-DMACOS=on .."
cmake -DCMAKE_BUILD_TYPE=$build $zlib -DSKIP_BUNDLES=$bundle $xmlparser $addressSanitizer -DUSE_VALIDATION_PARSER=$validationParser -DMACOS=on ..
make
