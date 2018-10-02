#!/bin/bash
# script to build on mac
build=MinSizeRel
dataCompressionLib=libcompression
bundle=off
xmlparserLib=applexml
xmlparser="-DXML_PARSER=applexml"

usage()
{
    echo "usage: makemac [-b buildType] [-xzlib] [-parser-xerces]"
    echo $'\t' "-b Build type. Default MinSizeRel"
    echo $'\t' "-xzlib Use MSIX SDK Zlib instead of inbox libCompression api. Default on MacOS is libCompression."
    echo $'\t' "-sb Skip bundle support."
    echo $'\t' "-parser-xerces Use xerces xml parser instead of default apple xml parser."
}

printsetup()
{
    echo "Build Type:" $build
    echo "Data Compression library:" $dataCompressionLib
    echo "Skip bundle support:" $bundle
    echo "parser:" $xmlparserLib
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
find . -name *msix* -d | xargs rm -r

cmake -DCMAKE_BUILD_TYPE=$build $zlib -DSKIP_BUNDLES=$bundle $xmlparser -DMACOS=on ..
make
