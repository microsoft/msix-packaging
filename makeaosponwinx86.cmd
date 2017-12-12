mkdir android
cd android

REM add NDK path to environment variable ANDROID_NDK_ROOT or ANDROID_NDK
REM or add -DCMAKE_ANDROID_NDK=/path/to/android-ndk in the cmake command
REM set path=%path%;c:\ninja
cmake -DCMAKE_ANDROID_NDK_TOOLCHAIN_VERSION=clang ^
    -DCMAKE_SYSTEM_NAME=Android ^
    -DCMAKE_SYSTEM_VERSION=27 ^
    -DCMAKE_ANDROID_ARCH_ABI=x86 ^
    -DCMAKE_ANDROID_STL_TYPE=c++_static ^
    -DCMAKE_BUILD_TYPE=Release ^
    -DAOSP=on ^
    -G"Ninja" ..

ninja