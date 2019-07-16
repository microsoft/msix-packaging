#!/bin/bash
# script to build on linux
build=MinSizeRel
bundle=off
validationParser=off
pack=off

usage()
{
    echo "usage: makelinux [options]"
    echo $'\t' "-b build_type           Default MinSizeRel"
    echo $'\t' "-sb                     Skip bundle support."
    echo $'\t' "--validation-parser|-vp Enable XML schema validation."
    echo $'\t' "--pack                  Include packaging features."
}

printsetup()
{
    echo "Build Type:" $build
    echo "Skip bundle support:" $bundle
    echo "Validation parser:" $validationParser
    echo "Pack support:" $pack 
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
        --validation-parser ) validationParser=on
                ;;
        -vp )   validationParser=on
                ;;
        --pack ) pack=on
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

echo "cmake -DCMAKE_BUILD_TYPE="$build "-DSKIP_BUNDLES="$bundle "-DUSE_VALIDATION_PARSER="$validationParser 
echo "-DCMAKE_TOOLCHAIN_FILE=../cmake/linux.cmake" "-DMSIX_PACK="$pack "-DLINUX=on .."
cmake -DCMAKE_BUILD_TYPE=$build \
      -DSKIP_BUNDLES=$bundle \
      -DUSE_VALIDATION_PARSER=$validationParser \
      -DCMAKE_TOOLCHAIN_FILE=../cmake/linux.cmake \
      -DMSIX_PACK=$pack \
      -DLINUX=on ..
make
