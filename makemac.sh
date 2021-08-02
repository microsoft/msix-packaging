#!/bin/bash
# script to build on mac
build=MinSizeRel
dataCompressionLib=libcompression
bundle=off
xmlparser=applexml
addressSanitizer=off
validationParser=off
pack=off
samples=on
tests=on
arch=x86_64

usage()
{
    echo "usage: makemac [options]"
    echo $'\t' "-b build_type           Default MinSizeRel"
    echo $'\t' "-xzlib                  Use MSIX SDK Zlib instead of inbox libCompression api. Default on MacOS is libCompression."
    echo $'\t' "-sb                     Skip bundle support."
    echo $'\t' "-parser-xerces          Use xerces xml parser instead of default apple xml parser."
    echo $'\t' "-asan                   Turn on address sanitizer for memory corruption detection."
    echo $'\t' "--validation-parser|-vp Enable XML schema validation."
    echo $'\t' "--pack                  Include packaging features. Uses MSIX SDK Zlib and Xerces with validation parser on."
    echo $'\t' "--skip-samples          Skip building samples."
    echo $'\t' "--skip-tests            Skip building tests."
    echo $'\t' "-arch arch              Architecture. Default x86_64"
}

printsetup()
{
    echo "Build Type:" $build
    echo "Data Compression library:" $dataCompressionLib
    echo "Skip bundle support:" $bundle
    echo "parser:" $xmlparser
    echo "Address Sanitizer:" $addressSanitizerFlag
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
        -xzlib )dataCompressionLib=MSIX_SDK_zlib
                zlib="-DUSE_MSIX_SDK_ZLIB=on"
                ;;
        -parser-xerces ) xmlparser=xerces
                         ;;
        -asan ) addressSanitizer=on
                ;;
        -sb )   bundle="on"
                ;;
        --validation-parser ) validationParser=on
                ;;
        -vp )   validationParser=on
                ;;
        --pack ) pack=on
                 dataCompressionLib=MSIX_SDK_zlib
                 zlib="-DUSE_MSIX_SDK_ZLIB=on"
                 xmlparser=xerces
                 validationParser=on
                ;;
        --skip-samples ) samples=off
                ;;
        --skip-tests ) tests=off
                ;;
        -h )    usage
                exit
                ;;
        -arch ) shift
                arch=$1
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

echo "cmake -DCMAKE_BUILD_TYPE="$build $zlib "-DSKIP_BUNDLES="$bundle 
echo "-DXML_PARSER="$xmlparser "-DASAN="$addressSanitizer "-DUSE_VALIDATION_PARSER="$validationParser 
echo "-DMSIX_PACK="$pack "-DMSIX_SAMPLES="$samples "-DMSIX_TESTS="$tests "-DCMAKE_OSX_ARCHITECTURES="$arch "-DMACOS=on .."
cmake -DCMAKE_BUILD_TYPE=$build \
      -DXML_PARSER=$xmlparser \
      -DSKIP_BUNDLES=$bundle \
      -DASAN=$addressSanitizer \
      -DUSE_VALIDATION_PARSER=$validationParser \
      -DMSIX_PACK=$pack \
      -DMSIX_SAMPLES=$samples \
      -DMSIX_TESTS=$tests \
      -DCMAKE_OSX_ARCHITECTURES=$arch \
      -DCMAKE_TOOLCHAIN_FILE=../cmake/macos.cmake \
      $zlib -DMACOS=on ..
make
