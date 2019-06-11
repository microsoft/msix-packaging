#!/bin/bash
testfailed=0
projectdir=`pwd`

emulatorName="msix_android_emulator"
avdPackage=""
install=0
outputFile=TEST-MsixSDK-AOSP.xml

usage()
{
    echo "usage: ./testaosponmac [-avd <emulator name>] [-c <package> [-i]]"
    echo $'\t' "-avd <emulator name>. Name of avd. Default msix_android_emulator"
    echo $'\t' "-c <package>. Create avd with specified package with the name defined by -adv. If not present assume the emulator already exits"
    echo $'\t' "-i . Only used with -c Installs the package specified on c"
    echo $'\t' "-o <name>. Name of output file. Default TEST-MsixSDK-AOSP.xml"
}

while [ "$1" != "" ]; do
    case $1 in
        -avd ) shift
             emulatorName=$1
             ;;
        -c ) shift
             avdPackage=$1
             ;;
        -i ) install=1
             ;;
        -h ) usage
             exit
             ;;
        -o ) shift
             outputFile=$1
             ;;
        * )  usage
             exit 1
    esac
    shift
done

function PrintFile {
    local FILE="$1"
    if [ ! -f $FILE ]
    then
        echo $FILE" not found!"
        exit 1
    fi
    cat $FILE
}

function TerminateEmulator {
    $ANDROID_HOME/platform-tools/adb emu kill &
}

# Clean up local result files if necessary
rm -f $outputFile

# Create emulator if requested
if [ -n "$avdPackage" ]; then
    if [ $install -ne 0 ]; then
        # Install AVD files
        echo "y" | $ANDROID_HOME/tools/bin/sdkmanager --install "$avdPackage"
    fi
    echo "Creating emulator" $emulatorName
    echo "no" | $ANDROID_HOME/tools/bin/avdmanager create avd -n $emulatorName -k "$avdPackage" --force
fi
if [ -z $($ANDROID_HOME/emulator/emulator -list-avds | grep "$emulatorName") ]; then
    echo "Emulator doesn't exits"
    exit 1
fi
echo "Starting emulator" $emulatorName
nohup $ANDROID_HOME/emulator/emulator -avd $emulatorName -no-snapshot -wipe-data > /dev/null 2>&1 &
$ANDROID_HOME/platform-tools/adb wait-for-device shell 'while [[ -z $(getprop sys.boot_completed) ]]; do sleep 1; done; input keyevent 82'
$ANDROID_HOME/platform-tools/adb devices
echo "Emulator started"

# Create App and copy resources and shared libraries.
cd $projectdir/AndroidBVT
# We always assume that .vs is the output dir. TODO: change if that assumptions isn't true
bindir=$projectdir/../../../.vs
mkdir -p app/src/main/assets
cp -R $projectdir/../testData app/src/main/assets
mkdir -p app/src/main/jniLibs/x86
cp $bindir/lib/libmsix.so app/src/main/jniLibs/x86
cp $bindir/msixtest/libmsixtest.so app/src/main/jniLibs/x86
mkdir -p app/src/main/libs
cp $bindir/lib/msix-jni.jar app/src/main/libs
cp $projectdir/../msixtest/inc/msixtest.hpp app/src/main/cpp/msixtest.hpp

rm -rf build app/build
sh ./gradlew assembleDebug

# Install app
$ANDROID_HOME/platform-tools/adb install -t -r app/build/outputs/apk/debug/app-debug.apk

# Clean up.test results in emulator if necesarry
$ANDROID_HOME/platform-tools/adb shell "rm -rf /storage/emulated/0/Android/data/com.microsoft.androidbvt/files/TEST-MsixSDK-AOSP.xml"

# Start app
$ANDROID_HOME/platform-tools/adb shell am start -n com.microsoft.androidbvt/.MainActivity

# The app terminates when is done
count=0
while ! $ANDROID_HOME/platform-tools/adb shell ps | grep -q "com.microsoft.androidbvt"
do
    echo "Waiting for test app to start..."
    ((count+=5))
    if [ $count -eq 120 ]; then
        echo "App never started"
        TerminateEmulator
        exit 1
    fi
    sleep 5
done
count=0
while adb shell ps | grep -q "com.microsoft.androidbvt"
do
    echo "Test is running..."
    ((count+=5))
    if [ $count -eq 240 ]; then
        echo "Test never completed"
        exit 1
    fi
    sleep 5
done
cd $projectdir

# Get Results
$ANDROID_HOME/platform-tools/adb pull /storage/emulated/0/Android/data/com.microsoft.androidbvt/files/TEST-MsixSDK-AOSP.xml $outputFile
echo "Tests completed"
TerminateEmulator

PrintFile $outputFile
