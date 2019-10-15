#!/bin/bash
# script to build on linux
build=MinSizeRel
bundle=off
validationParser=off
pack=off
samples=on
tests=on

usage()
{
    echo "usage: makelinux [options]"
    echo $'\t' "-b build_type           Default MinSizeRel"
    echo $'\t' "-sb                     Skip bundle support."
    echo $'\t' "--validation-parser|-vp Enable XML schema validation."
    echo $'\t' "--pack                  Include packaging features. Sets validation parser on."
    echo $'\t' "--skip-samples          Skip building samples."
    echo $'\t' "--skip-tests            Skip building tests."
}

printsetup()
{
    echo "Build Type:" $build
    echo "Skip bundle support:" $bundle
    echo "Validation parser:" $validationParser
    echo "Pack support:" $pack 
    echo "Build samples:" $samples
    echo "Build tests:" $tests
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
                 validationParser=on
                 ;;
        --skip-samples ) samples=off
                ;;
        --skip-tests ) tests=off
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
echo "-DCMAKE_TOOLCHAIN_FILE=../cmake/linux.cmake" "-DMSIX_PACK="$pack "-DMSIX_SAMPLES="$samples "-DMSIX__TESTS="$tests "-DLINUX=on .."
cmake -DCMAKE_BUILD_TYPE=$build \
      -DSKIP_BUNDLES=$bundle \
      -DUSE_VALIDATION_PARSER=$validationParser \
      -DCMAKE_TOOLCHAIN_FILE=../cmake/linux.cmake \
      -DMSIX_PACK=$pack \
      -DMSIX_SAMPLES=$samples \
      -DMSIX_TESTS=$tests \
      -DLINUX=on ..
make
