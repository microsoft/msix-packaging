//
//  AppDelegate.m
//  iOSBVT
//
//  Copyright (C) 2017 Microsoft.  All rights reserved.
//  See LICENSE file in the project root for full license information.
// 

#import "AppDelegate.h"
#import "msixtest.hpp"

@interface AppDelegate ()

@end

@implementation AppDelegate


- (BOOL)application:(UIApplication *)application didFinishLaunchingWithOptions:(NSDictionary *)launchOptions {
    // Override point for customization after application launch.
    
    char str[256] = {0};
    NSBundle* mainBundle = [NSBundle mainBundle];
    NSString* resourcePath = mainBundle.resourcePath;
    const char* testDataPath = [resourcePath UTF8String];

    // msixtests
    // Simulate commnad arguments: "msixtest -s -r junit -o tmp/TEST-MsixSDK-iOS.xml"
    char* arguments[6] = { "msix_test", "-s", "-r", "junit" , "-o", "tmp/TEST-MsixSDK-iOS.xml" };
    // Usefull commands for testing
    //char *arguments[2] = { "msix_test", "-l" };
    //char *arguments[2] = { "msix_test", "<name of test>" };
    int result = msixtest_main(6, arguments, testDataPath);
    sprintf(str,"%d", result);
    exit(0); // Maybe I wouldn't do this if there was an actual API to do this easily...
    return YES;
}


- (void)applicationWillResignActive:(UIApplication *)application {
    // Sent when the application is about to move from active to inactive state. This can occur for certain types of temporary interruptions (such as an incoming phone call or SMS message) or when the user quits the application and it begins the transition to the background state.
    // Use this method to pause ongoing tasks, disable timers, and invalidate graphics rendering callbacks. Games should use this method to pause the game.
}


- (void)applicationDidEnterBackground:(UIApplication *)application {
    // Use this method to release shared resources, save user data, invalidate timers, and store enough application state information to restore your application to its current state in case it is terminated later.
    // If your application supports background execution, this method is called instead of applicationWillTerminate: when the user quits.
}


- (void)applicationWillEnterForeground:(UIApplication *)application {
    // Called as part of the transition from the background to the active state; here you can undo many of the changes made on entering the background.
}


- (void)applicationDidBecomeActive:(UIApplication *)application {
    // Restart any tasks that were paused (or not yet started) while the application was inactive. If the application was previously in the background, optionally refresh the user interface.
}


- (void)applicationWillTerminate:(UIApplication *)application {
    // Called when the application is about to terminate. Save data if appropriate. See also applicationDidEnterBackground:.
}


@end
