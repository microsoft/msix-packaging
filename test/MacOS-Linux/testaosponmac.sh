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
}

function ParseResult {
    if [ ! -f testResults.txt ]
    then
        echo "testResults.txt not found!"
        exit 1
    fi
    cat testResults.txt 
    if grep -q "passed" testResults.txt
    then
        echo "Android tests passed"
        exit 0
    else
        echo "Android tests failed."
        exit 1
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

CreateApp
RunTest
TerminateEmulatorInBackground
ParseResult
