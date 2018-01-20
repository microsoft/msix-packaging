#!/bin/bash
testfailed=0
xplatappxdir=`pwd`

function RunCommandWithTimeout {
	local result=1
	local waitingtime=0
	while [ $result -ne 0 ]
	do
		waitingtime=$((waitingtime++))
		if [ $waitingtime -eq 30 ]
		then
			echo "Time out"
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
	cd $xplatappxdir/test/mobile/xPlatAppxAndroid
	mkdir app/src/main/assets
	cp -R $xplatappxdir/test/appx/* app/src/main/assets
	mkdir -p app/src/main/jniLibs/x86
	cp $xplatappxdir/android/lib/libxPlatAppx.so app/src/main/jniLibs/x86
	JAVA_HOME="/Applications/Android Studio.app/Contents/jre/jdk/Contents/Home"
	rm -r build
	sh ./gradlew assembleDebug
}

function RunTest {
	# Install app
	RunCommand "adb push app/build/outputs/apk/debug/app-debug.apk /data/local/tmp/com.microsoft.xplatappxandroid"
	RunCommand "adb shell pm install -t -r '/data/local/tmp/com.microsoft.xplatappxandroid'"
	# Start app
	RunCommand "adb shell am start -n 'com.microsoft.xplatappxandroid/com.microsoft.xplatappxandroid.MainActivity' -a android.intent.action.MAIN -c android.intent.category.LAUNCHER"
	# The apps terminates when is done
	sleep 10
	# Get Results
	RunCommand "adb pull /data/data/com.microsoft.xplatappxandroid/files/testResults.txt"
}

function ParseResult {
	if [ ! -f testResults.txt ]
	then
		echo "testResults.txt not found!"
		exit 1
	fi
	if grep -q "passed" testResults.txt
	then
		echo "Android tests passed"
		exit 0
	else
		echo "Android tests failed."
		less testResults.txt 
		exit 1
	fi
}

StartEmulator
# Clean up. This commands might fail, but is not an error
adb shell rm -r /data/data/com.microsoft.xplatappxandroid/files
rm $xplatappxdir/test/mobile/xPlatAppxAndroid/testResults.txt

CreateApp
RunTest

# Turn off the emulator
adb emu kill
ParseResult
