#!/bin/bash
# script to build on mac
build=MinSizeRel
dataCompressionLib=libcompression

usage()
{
    echo "usage: makemac [-b buildType] [-zlib]"
    echo $'\t' "-b Build type. Default MinSizeRel"
    echo $'\t' "-zlib Use Zlib instead of inbox libCompression api. Default on MacOS is libCompression."
}

printsetup()
{
    echo "Build Type:" $build
    echo "Data Compression library:" $dataCompressionLib
}

while [ "$1" != "" ]; do
    case $1 in
        -b )    shift
                build=$1
                ;;
        -zlib ) shift
                dataCompressionLib=zlib
                zlib="-DUSE_ZLIB=on"
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

cmake -DCMAKE_BUILD_TYPE=$build $zlib -DMACOS=on ..
make
