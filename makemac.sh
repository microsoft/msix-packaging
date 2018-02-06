#!/bin/bash
# script to build on mac
build=MinSizeRel

usage()
{
    echo "usage: makemac [-b buildType]"
    echo $'\t' "-b Build type. Default MinSizeRel"
}

printsetup()
{
    echo "Build Type:" $build
}

while [ "$1" != "" ]; do
    case $1 in
        -b )    shift
                build=$1
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

# clean up any old builds of xPlatAppx modules
find . -name *xPlatAppx* -d | xargs rm -r

cmake -DCMAKE_BUILD_TYPE=$build -DMACOS=on ..
make
