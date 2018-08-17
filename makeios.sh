#!/bin/bash
# script to build on mac for ios
build=MinSizeRel
arch=x86_64
dataCompressionLib=libcompression

usage()
{
    echo "usage: makemac [-b buildType] [-arch] [-zlib]"
    echo $'\t' "-b Build type. Default MinSizeRel"
    echo $'\t' "-arch OSX Architecture. Default x86_64 (simulator)"
    echo $'\t' "-zlib Use Zlib instead of inbox libCompression api. Default on iOS is libCompression."
}

printsetup()
{
    echo "Build Type:" $build
    echo "Architecture:" $arch
    echo "Data Compression library:" $dataCompressionLib
}

while [ "$1" != "" ]; do
    case $1 in
        -b )    shift
                build=$1
                ;;
        -arch ) shift
                arch=$1
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

cmake -DCMAKE_BUILD_TYPE=$build $zlib -DIOS=on -DCMAKE_TOOLCHAIN_FILE=../cmake/ios.cmake -DCMAKE_OSX_ARCHITECTURES=$arch ..
make
