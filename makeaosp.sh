#!/bin/bash
# script to build AOSP.

ndk=
arch=x86
build=MinSizeRel
version=19

usage()
{
    echo "usage: makeaosp [-ndk ndk_path] [-arch arch] [-v version] [-b buildType]"
    echo $'\t' "-ndk Path to Android NDK. Default $ANDROID_NDK_ROOT or $ANDROID_NDK"
    echo $'\t' "-arch Architecture ABI. Default x86"
    echo $'\t' "-v Android version. Default 19"
    echo $'\t' "-b Build type. Default MinSizeRel"
}

printsetup()
{
    echo "NDK Path:" $ndk
    echo "Architecture:" $arch
    echo "Version:" $version
    echo "Build Type:" $build
}

while [ "$1" != "" ]; do
    case $1 in
        -ndk )  shift
                ndk=$1
                ;;
        -arch ) shift
                arch=$1
                ;;
        -b )    shift
                build=$1
                ;;
        -v )    shift
                version=$1
                ;;
        -h )    usage
                exit
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
printsetup

mkdir .vs
cd .vs

# clean up any old builds of xPlatAppx modules
find . -name *xPlatAppx* -d | xargs rm -r

cmake -DCMAKE_SYSTEM_NAME=Android \
    -DCMAKE_ANDROID_NDK="$ndk" \
    -DCMAKE_SYSTEM_VERSION="$version" \
    -DCMAKE_ANDROID_ARCH_ABI="$arch" \
    -DCMAKE_ANDROID_NDK_TOOLCHAIN_VERSION=clang \
    -DCMAKE_ANDROID_STL_TYPE=c++_shared \
    -DCMAKE_BUILD_TYPE="$build" -DAOSP=on ..
make
