#!/bin/bash
testfailed=0
device="iPhone 8"
project=../mobile/iOSBVT/iOSBVT.xcodeproj
sdk=iphonesimulator

usage()
{
    echo "usage: ./testios [-d <device>] [-p <project>] [-sdk <sdk>] [-o <ouput location>]"
    echo $'\t' "-d <device>. Default iPhone 8"
    echo $'\t' "-p <project>. Default is ../mobile/iOSBVT/iOSBVT.xcodeproj"
    echo $'\t' "-sdk <sdk>. Default is iphonesimulator"
    echo $'\t' "-o <output file>. Test result output file. Default is /private/tmp/testResults.txt"
}

printsetup()
{
    echo "Device: " $device
    echo "Project: " $project
    echo "App Name: " $app
    echo "AppIdentifier: " $appId
    echo "Sdk: " $sdk
    echo "Output file Location: " $resultFile
}

while [ "$1" != "" ]; do
    case $1 in
        -d ) shift
             device=$1
             ;;
        -p ) shift
             project=$1
             ;;
        -sdk ) shift
               sdk=$1
               ;;
        -o ) shift
             outputFile=$1
             ;;
        -h ) usage
             exit
             ;;
        * )  usage
             exit 1
    esac
    shift
done

# Get app name and the bundle identifier
# We are looking for something like "PRODUCT_MODULE_NAME = <appName>" and "PRODUCT_BUNDLE_IDENTIFIER = <appId>"
app=`xcodebuild -showBuildSettings -project $project | grep PRODUCT_MODULE_NAME | awk '{print $3}'`
appId=`xcodebuild -showBuildSettings -project $project | grep PRODUCT_BUNDLE_IDENTIFIER | awk '{print $3}'`

printsetup

# Clean test
rm $outputFile -f

# Start emulator
echo "Starting emulator"
xcrun simctl boot "$device"

# Build app and locate .app to install
xcodebuild -scheme $app -project $project -sdk $sdk
build_result=$?
if [ $build_result -ne 0 ]
then
    exit 1
fi

# We are looking for something like "BUILD_ROOT = <appDir>"
appDir=`xcodebuild -showBuildSettings -project $project | grep BUILD_ROOT | awk '{print $3}'`
appDir=$appDir/Debug-iphonesimulator/$app.app
echo Installing app $appDir

# Install app. We don't need to wait for the emulator to start
# once is booted. This command will wait until the instalation
# succeeds or fail even if the emulator is starting up.
echo "Installing app " $app
xcrun simctl install booted $appDir

# Run app and get process
xcrun simctl launch booted $appId &

# Look for a process where its command contains <App>.app
# If we find it the app is running, if we don't the app hasn't started yet.
# The first grep will produce two results so we need to explicitly remove "grep"
while ! ps aux | grep $app.app | grep -v grep
do
    echo "Waiting for test app to start..."
    sleep 5
done
echo "Test app started"
while ps aux | grep $app.app | grep -v grep
do
    echo "Waiting for test to finish..."
    sleep 5
done
echo "Test finished"

echo "Uninstalling app and shuting down emulator"
xcrun simctl uninstall booted $appId
xcrun simctl shutdown "$device"

function ParseResult {
    local FILE="$1"
    if [ ! -f $FILE ]
    then
        echo $FILE" not found!"
        exit 1
    fi
    cat $FILE
    if grep -q "FAILED" $FILE
    then
        echo "FAILED"
        testfailed=1
    else
        echo "succeeded"
    fi
}

ParseResult /private/tmp/testResults.txt
ParseResult /private/tmp/testApiResults.txt

echo "-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-="
if [ $testfailed -ne 0 ]
then
    echo "                           FAILED                                 "
    exit $testfailed
else
    echo "                           passed                                 "
    exit 0
fi
