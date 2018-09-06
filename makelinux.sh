#!/bin/bash
# script to build on linux
build=MinSizeRel
bundle=off

usage()
{
    echo "usage: makelinux [-b buildType] [-sb]"
    echo $'\t' "-b Build type. Default MinSizeRel"
    echo $'\t' "-sb Skip bundle support."
}

printsetup()
{
    echo "Build Type:" $build
    echo "Skip bundle support:" $bundle
}

while [ "$1" != "" ]; do
    case $1 in
        -b )    shift
                build=$1
                ;;
        -h )    usage
                exit
                ;;
        -sb )   bundle="on"
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

cmake -DCMAKE_BUILD_TYPE=$build -DSKIP_BUNDLES=$bundle -DLINUX=on ..
make
