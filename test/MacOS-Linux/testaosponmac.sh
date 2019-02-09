#!/bin/bash
testfailed=0
projectdir=`pwd`

function RunCommandWithTimeout {
    local result=1
    local waitingtime=0
    while [ $result -ne 0 ]
    do
        waitingtime=$((waitingtime++))
        if [ $waitingtime -eq 30 ]
        then
            echo "Time out"
            TerminateEmulatorInBackground
            exit 1
        fi
        sleep 1
        echo "Waiting for Android emulator to start"
        $1
        result=$?
    done
}

function RunCommand {
    $1
    local result=$?
    if [ $result -ne 0 ]
    then
        echo "Setup failure"
        TerminateEmulatorInBackground
        exit 1
    fi
}

function StartEmulator {
    cd $ANDROID_HOME/tools
    emulator -avd Nexus_5X_API_19_x86 -netdelay none -netspeed full &
    RunCommandWithTimeout "adb shell getprop dev.bootcomplete"
    RunCommandWithTimeout "adb shell getprop init.svc.bootanim"
    # At this time the device booted, but give some time to stabilize
    sleep 10
    echo "Android emulator started"
}

function CreateApp {
    # Prepare package and compile
    cd $projectdir/../mobile/AndroidBVT
    mkdir -p app/src/main/assets
    cp -R $projectdir/../appx/* app/src/main/assets
    cp $projectdir/../../.vs/test/api/input/apitest_test_1.txt app/src/main/assets
    mkdir -p app/src/main/jniLibs/x86
    cp $projectdir/../../.vs/lib/libmsix.so app/src/main/jniLibs/x86
    mkdir -p app/src/main/libs
    cp $projectdir/../../.vs/lib/msix-jni.jar app/src/main/libs
    rm -r build app/build
    sh ./gradlew assembleDebug
}

function RunTest {
    # Install app
    RunCommand "adb push app/build/outputs/apk/debug/app-debug.apk /data/local/tmp/com.microsoft.androidbvt"
    RunCommand "adb shell pm install -t -r '/data/local/tmp/com.microsoft.androidbvt'"
    # Start app
    RunCommand "adb shell am start -n 'com.microsoft.androidbvt/com.microsoft.androidbvt.MainActivity' -a android.intent.action.MAIN -c android.intent.category.LAUNCHER"
    # The apps terminates when is done
    while ! adb shell ps | grep -q "com.microsoft.androidbvt"
    do
        echo "Waiting for test app to start..."
        sleep 5
    done
    while adb shell ps | grep -q "com.microsoft.androidbvt"
    do
        echo "Test is running..."
        sleep 5
    done
    # Get Results
    RunCommand "adb pull /data/data/com.microsoft.androidbvt/files/testResults.txt"
    RunCommand "adb pull /data/data/com.microsoft.androidbvt/files/testApiResults.txt"
}

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

# Terminate the emulator best effort
function TerminateEmulatorInBackground {
    adb emu kill &
}

StartEmulator
# Clean up. This commands might fail, but is not an error
adb shell rm -r /data/data/com.microsoft.androidbvt/files
rm $projectdir/../mobile/androidbvt/testResults.txt
rm $projectdir/../mobile/androidbvt/testApiResults.txt

CreateApp
RunTest
TerminateEmulatorInBackground
ParseResult testResults.txt
ParseResult testApiResults.txt

echo "-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-="
if [ $testfailed -ne 0 ]
then
    echo "                           FAILED                                 "
    exit $testfailed
else
    echo "                           passed                                 "
    exit 0
fi
