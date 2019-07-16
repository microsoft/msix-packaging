#!/bin/bash
# script to build on mac
build=MinSizeRel
dataCompressionLib=libcompression
bundle=off
xmlparserLib=applexml
addressSanitizer=off
validationParser=off
pack=on

usage()
{
    echo "usage: makemac [options]"
    echo $'\t' "-b build_type           Default MinSizeRel"
    echo $'\t' "-xzlib                  Use MSIX SDK Zlib instead of inbox libCompression api. Default on MacOS is libCompression. Required for pack support."
    echo $'\t' "-sb                     Skip bundle support."
    echo $'\t' "-parser-xerces          Use xerces xml parser instead of default apple xml parser."
    echo $'\t' "-asan                   Turn on address sanitizer for memory corruption detection."
    echo $'\t' "--validation-parser|-vp Enable XML schema validation."
    echo $'\t' "--no-pack               Don't include packaging features."
}

printsetup()
{
    echo "Build Type:" $build
    echo "Data Compression library:" $dataCompressionLib
    echo "Skip bundle support:" $bundle
    echo "parser:" $xmlparserLib
    echo "Address Sanitizer:" $addressSanitizerFlag
    echo "Validation parser:" $validationParser
    echo "Pack support:" $pack 
}

while [ "$1" != "" ]; do
    case $1 in
        -b )    shift
                build=$1
                ;;
        -xzlib )dataCompressionLib=MSIX_SDK_zlib
                zlib="-DUSE_MSIX_SDK_ZLIB=on"
                ;;
        -parser-xerces ) xmlparserLib=xerces
                         xmlparser="-DXML_PARSER=xerces"
                         ;;
        -asan ) addressSanitizer=on
                ;;
        -sb )   bundle="on"
                ;;
        --validation-parser ) validationParser=on
                ;;
        -vp )   validationParser=on
                ;;
        --no-pack ) pack=off
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

echo "cmake -DCMAKE_BUILD_TYPE="$build $zlib "-DSKIP_BUNDLES="$bundle 
echo $xmlparser "-DASAN="$addressSanitizer "-DUSE_VALIDATION_PARSER="$validationParser 
echo "-DMSIX_PACK="$pack "-DMACOS=on .."
cmake -DCMAKE_BUILD_TYPE=$build \
      $xmlparser \
      -DSKIP_BUNDLES=$bundle \
      -DASAN=$addressSanitizer \
      -DUSE_VALIDATION_PARSER=$validationParser \
      -DMSIX_PACK=$pack \
      $zlib -DMACOS=on ..
make
