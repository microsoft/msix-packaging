#!/bin/bash
# script to build AOSP.

ndk=
arch=x86
build=MinSizeRel
version=24
sdk=
sdkver=34
dataCompressionLib=NDK_libz
bundle=off
xmlparser=javaxml
validationParser=off
samples=on
tests=on

usage()
{
    echo "usage: makeaosp [options]"
    echo $'\t' "-ndk ndk_path           Path to Android NDK. Default $ANDROID_NDK_ROOT or $ANDROID_NDK"
    echo $'\t' "-ndkver ndk_version     Android NDK version. Default/minimum 19."
    echo $'\t' "-sdk sdk_path           Path to Android SDK. Default $ANDROID_HOME."
    echo $'\t' "-sdkver sdk_version     Android SDK version. Default/minimum 24."
    echo $'\t' "-arch arch              Architecture ABI. Default x86"
    echo $'\t' "-b build_type           Default MinSizeRel"
    echo $'\t' "-xzlib                  Use MSIX SDK Zlib instead of inbox libz.so"
    echo $'\t' "-parser-xerces          Use xerces xml parser instead of default javaxml"
    echo $'\t' "-sb                     Skip bundle support."
    echo $'\t' "--validation-parser|-vp Enable XML schema validation."
    echo $'\t' "--skip-samples          Skip building samples."
    echo $'\t' "--skip-tests            Skip building tests."
}

printsetup()
{
    echo "NDK Path:" $ndk
    echo "NDK Version:" $version
    echo "SDK Path:" $sdk
    echo "SDK Version:" $sdkver
    echo "Architecture:" $arch
    echo "Build Type:" $build
    echo "Zlib:" $dataCompressionLib
    echo "parser:" $xmlparser
    echo "Skip bundle support:" $bundle
    echo "Validation parser:" $validationParser
    echo "Build samples:" $samples
    echo "Build tests:" $tests
}

while [ "$1" != "" ]; do
    case $1 in
        -ndk )    shift
                  ndk=$1
                  ;;
        -arch )   shift
                  arch=$1
                  ;;
        -b )      shift
                  build=$1
                  ;;
        -ndkver ) shift
                  version=$1
                  ;;
        -h )      usage
                  exit
                  ;;
        -xzlib )  dataCompressionLib=MSIX_SDK_zlib
                  zlib="-DUSE_MSIX_SDK_ZLIB=on"
                  ;;
        -parser-xerces )
                  xmlparser=xerces
                  ;;
        -sdk )    shift
                  sdk=$1
                  ;;
        -sdkver ) shift
                  sdkver=$1
                  ;;
        -sb )     bundle="on"
                  ;;
        --validation-parser ) validationParser=on
                ;;
        -vp )   validationParser=on
                ;;
        --skip-samples ) samples=off
                ;;
        --skip-tests ) tests=off
                ;;
        * )       usage
                  exit 1
    esac
    shift
done

if [ -z "$sdk" ] && [ -n "$ANDROID_HOME" ]; then
    sdk="$ANDROID_HOME"
elif [ -z "$sdk" ]; then
    echo "Android SDK not found"
    exit 1
fi

if [ -z "$ndk" ] && [ -n "$ANDROID_NDK_ROOT" ]; then
    ndk="$ANDROID_NDK_ROOT"
elif [ -z "$ndk" ] && [ -n "$ANDROID_ROOT"]; then
    ndk="$ANDROID_ROOT"
fi

# If we find the sdk and ndk is still empty lets just hope they have it
# installed in the default location.
# Note: don't elif this to the block above as I've seen ANDROID_NDK_ROOT or 
# ANDROID_ROOT set but empty.
if [ -z "$ndk" ]; then
    ndk="$ANDROID_HOME/ndk-bundle"
fi

printsetup

mkdir .vs
cd .vs

# clean up any old builds of msix modules
find . -name *msix* -d | xargs rm -r

echo "cmake -DCMAKE_SYSTEM_NAME=Android -DCMAKE_ANDROID_NDK="$ndk "-DCMAKE_SYSTEM_VERSION="$version "-DANDROID_SDK="$sdk
echo "-DANDROID_SDK_VERSION="$sdkver "-DCMAKE_ANDROID_ARCH_ABI="$arch "-DCMAKE_ANDROID_NDK_TOOLCHAIN_VERSION=clang"
echo "-DCMAKE_ANDROID_STL_TYPE=c++_shared -DCMAKE_BUILD_TYPE="$build "-DSKIP_BUNDLES="$bundle "-DXML_PARSER="$xmlparser
echo "-DUSE_VALIDATION_PARSER="$validationParser $zlib "-DMSIX_SAMPLES="$samples "-DMSIX_TESTS="$tests "-DAOSP=on .."
cmake -DCMAKE_SYSTEM_NAME=Android \
    -DCMAKE_ANDROID_NDK="$ndk" \
    -DCMAKE_SYSTEM_VERSION="$version" \
    -DANDROID_SDK="$sdk" \
    -DANDROID_SDK_VERSION="$sdkver" \
    -DCMAKE_ANDROID_ARCH_ABI="$arch" \
    -DCMAKE_ANDROID_NDK_TOOLCHAIN_VERSION=clang \
    -DCMAKE_ANDROID_STL_TYPE=c++_shared \
    -DCMAKE_BUILD_TYPE="$build" \
    -DSKIP_BUNDLES=$bundle \
    -DXML_PARSER=$xmlparser \
    -DUSE_VALIDATION_PARSER=$validationParser \
    -DMSIX_SAMPLES=$samples \
    -DMSIX_TESTS=$tests \
    $zlib -DAOSP=on ..
make
