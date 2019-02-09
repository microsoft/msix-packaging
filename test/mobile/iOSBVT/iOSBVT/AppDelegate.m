//
//  AppDelegate.m
//  iOSBVT
//
//  Copyright (C) 2017 Microsoft.  All rights reserved.
//  See LICENSE file in the project root for full license information.
// 

#import "AppDelegate.h"
extern signed long RunTests(char* source, char* target);
extern int RunApiTest(char* input, char* target, char* packageRootPath);
@interface AppDelegate ()

@end

@implementation AppDelegate


- (BOOL)application:(UIApplication *)application didFinishLaunchingWithOptions:(NSDictionary *)launchOptions {
    // Override point for customization after application launch.
    
    char str[256] = {0};
    NSBundle* mainBundle = [NSBundle mainBundle];
    NSString* resourcePath = mainBundle.resourcePath;
    NSString* sourcePath = [resourcePath stringByAppendingString:@"/"];
    // End-to-end tests
    char* source = [sourcePath UTF8String];
    unsigned long result = RunTests(source, "tmp/");
    sprintf(str,"0x%08X", result);
    // Api tests
    NSString* inputFile = [sourcePath stringByAppendingString:@"apitest_test_1.txt"];
    char* input = [inputFile UTF8String];
    int apiResult = RunApiTest(input, "tmp/", source);
    sprintf(str,"%d", apiResult);
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
