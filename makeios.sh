#!/bin/bash
# script to build on mac for ios
build=MinSizeRel
arch=x86_64
dataCompressionLib=libcompression
bundle=off
xmlparser=applexml
validationParser=off
samples=on
tests=on

usage()
{
    echo "usage: makemac [options]"
    echo $'\t' "-b build_type           Default MinSizeRel"
    echo $'\t' "-arch arch              OSX Architecture. Default x86_64 (simulator)"
    echo $'\t' "-xzlib                  Use MSIX SDK Zlib instead of inbox libCompression api. Default on iOS is libCompression."
    echo $'\t' "-sb                     Skip bundle support."
    echo $'\t' "-parser-xerces          Use xerces xml parser instead of default apple xml parser."
    echo $'\t' "--validation-parser|-vp Enable XML schema validation."
    echo $'\t' "--skip-samples          Skip building samples."
    echo $'\t' "--skip-tests            Skip building tests."
}

printsetup()
{
    echo "Build Type:" $build
    echo "Architecture:" $arch
    echo "Data Compression library:" $dataCompressionLib
    echo "Skip bundle support:" $bundle
    echo "Parser:" $xmlparser
    echo "Validation parser:" $validationParser
    echo "Build samples:" $samples
    echo "Build tests:" $tests
}

while [ "$1" != "" ]; do
    case $1 in
        -b )    shift
                build=$1
                ;;
        -arch ) shift
                arch=$1
                ;;
        -xzlib )dataCompressionLib=MSIX_SDK_zlib
                zlib="-DUSE_MSIX_SDK_ZLIB=on"
                ;;
        -parser-xerces )  xmlparserLib=xerces
                xmlparser=xerces
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
find . -name *msix* -d | xargs rm -r

echo "cmake -DCMAKE_BUILD_TYPE="$build $zlib "-DCMAKE_TOOLCHAIN_FILE=../cmake/ios.cmake -DCMAKE_OSX_ARCHITECTURES="$arch 
echo "-DXML_PARSER="$xmlparser "-DUSE_VALIDATION_PARSER="$validationParser "-DSKIP_BUNDLES="$bundle "-DSKIP_SAMPLES="$samples "-DSKIP_TESTS="$tests  "-DIOS=on .."
cmake -DCMAKE_BUILD_TYPE=$build \
    -DCMAKE_TOOLCHAIN_FILE=../cmake/ios.cmake \
    -DCMAKE_OSX_ARCHITECTURES=$arch \
    -DXML_PARSER=$xmlparser \
    -DUSE_VALIDATION_PARSER=$validationParser \
    -DSKIP_BUNDLES=$bundle \
    -DMSIX_SAMPLES=$samples \
    -DMSIX_TESTS=$tests \
    $zlib -DIOS=on .. 
make
