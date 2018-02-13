#!/bin/bash
# script to build on mac for ios
build=MinSizeRel
arch=x86_64

usage()
{
    echo "usage: makemac [-b buildType]"
    echo $'\t' "-b Build type. Default MinSizeRel"
    echo $'\t' "-arch OSX Architecture. Default x86_64 (simulator)"
}

printsetup()
{
    echo "Build Type:" $build
    echo "Architecture:" $arch
}

while [ "$1" != "" ]; do
    case $1 in
        -b )    shift
                build=$1
                ;;
        -arch ) shift
                arch=$1
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

cmake -DCMAKE_BUILD_TYPE=$build -DIOS=on -DCMAKE_TOOLCHAIN_FILE=../cmake/ios.cmake -DCMAKE_OSX_ARCHITECTURES=$arch ..
make