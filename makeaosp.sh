#!/bin/bash
# script to build AOSP.

ndk=
arch=x86
build=MinSizeRel
version=19
sdk=
sdkver=24

usage()
{
    echo "usage: makeaosp [-ndk ndk_path] [-arch arch] [-ndkver ndk_version] [-sdk sdk_path] [-sdkver sdk_version] [-b buildType]"
    echo $'\t' "-ndk Path to Android NDK. Default $ANDROID_NDK_ROOT or $ANDROID_NDK"
    echo $'\t' "-ndkver Android NDK version. Default/minimum 19."
    echo $'\t' "-sdk Path to Android SDK. Default $ANDROID_HOME."
    echo $'\t' "-sdkver Android SDK version. Default/minimum 24."
    echo $'\t' "-arch Architecture ABI. Default x86"
    echo $'\t' "-b Build type. Default MinSizeRel"
}

printsetup()
{
    echo "NDK Path:" $ndk
    echo "NDK Version:" $version
    echo "SDK Path:" $sdk
    echo "SDK Version:" $sdkver
    echo "Architecture:" $arch
    echo "Build Type:" $build
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
        -sdk )    shift
                  sdk=$1
                  ;;
        -sdkver ) shift
                  sdkver=$1
                  ;;
        * )     usage
                exit 1
    esac
    shift
done

if [ -z "$ndk" ] && [ -n "$ANDROID_NDK_ROOT" ]; then
    ndk="$ANDROID_NDK_ROOT"
elif [ -z "$ndk" ] && [ -n "$ANDROID_ROOT"]; then
    ndk="$ANDROID_ROOT"
elif [ -z "$ndk" ]; then
    echo "Android NDK not found"
    exit 1
fi

if [ -z "$sdk" ] && [ -n "$ANDROID_HOME" ]; then
    sdk="$ANDROID_HOME"
elif [ -z "$sdk" ]; then
    echo "Android SDK not found"
    exit 1
fi

printsetup

mkdir .vs
cd .vs

# clean up any old builds of msix modules
find . -name *msix* -d | xargs rm -r

cmake -DCMAKE_SYSTEM_NAME=Android \
    -DCMAKE_ANDROID_NDK="$ndk" \
    -DCMAKE_SYSTEM_VERSION="$version" \
    -DANDROID_SDK="$sdk" \
    -DANDROID_SDK_VERSION="$sdkver" \
    -DCMAKE_ANDROID_ARCH_ABI="$arch" \
    -DCMAKE_ANDROID_NDK_TOOLCHAIN_VERSION=clang \
    -DCMAKE_ANDROID_STL_TYPE=c++_shared \
    -DCMAKE_BUILD_TYPE="$build" -DAOSP=on ..
make
