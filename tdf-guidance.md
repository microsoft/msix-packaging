
#Developer Guidance to target MSIX Packaging SDK

## Build a package to target the SDK 

The MSIX Packaging SDK offers developers a universal way to distribute package contents to client devices regardless of the OS platform on the client device. This enables developers to package their app content once instead of having to package for each platform. 

To take advantage of the new SDK and the ability to distribute your package contents to multiple platforms, we are providing a way to specify the target platforms where you want your packages to extract to. This means, the app developers can ensure that the package contents are being extracted from the package only as desired. 

The following table shows the target device families to declare in the manifest:

| **Platform**   | **Family**      | **Target Device Family**|                    |                        | **Notes**                                                      |
|:------------:|:-------------:|:----------------------:|:--------------------:|:------------------------:|:------------------------------------------------------------:|
| Windows 10 | Phone       | Platform.All         | Windows.Universal  | Windows.Mobile         | Mobile devices                                             |
|            | Desktop     |                      |                    | Windows.Desktop        | PC                                                         |
|            | Xbox        |                      |                    | Windows.Xbox           | Xbox console                                               |
|            | Surface Hub |                      |                    | Windows.Team           | Large screen Win 10 devices                                |
|            | HoloLens    |                      |                    | Windows.Holographic    | VR/AR headset                                              |
|            | IoT         |                      |                    | Windows.IoT            | IoT devices                                                |
| iOS        | Phone       |                      | Apple.Ios.All      | Apple.Ios.Phone        | Touch, iPhone                                              |
|            | Tablet      |                      |                    | Apple.Ios.Tablet       | iPad mini, iPad, iPad Pro                                  |
|            | TV          |                      |                    | Apple.Ios.TV           | Apple TV                                                   |
|            | Watch       |                      |                    | Apple.Ios.Watch        | iWatch                                                     |
| MacOS      | Desktop     |                      |                    | Apple.MacOS.Desktop    | MacBook Pro, MacBook Air, Mac Mini, iMac                   |
| Android    | Phone       |                      | Google.Android.All | Google.Android.Phone   | Pixels, galaxy etc. These can target any flavor of Android |
|            | Tablet      |                      |                    | Google.Android.Tablet  | Android tablets                                            |
|            | Desktop     |                      |                    | Google.Android.Desktop | Chromebooks                                                |
|            | TV          |                      |                    | Google.Android.TV      |                                                            |
|            | Watch       |                      |                    | Google.Android.Watch   | Google gear devices                                        |
| Windows    | 7           |                      | Windows7.Desktop   |                        | Windows 7 devices                                          |
|            | 8           |                      | Windows8.Desktop   |                        | Windows 8/8.1 devices                                      |
| Web        | Microsoft   |                      | Web.All            | Web.Edge.All           | edge web engine apps                                       |
|            | Android     |                      |                    | Web.Blink.All          | Blink web engine apps                                      |
|            | Apple       |                      |                    | Web.Webkit.All         | Webkit web engine apps                                     |
|            | Chrome      |                      |                    | Web.Chromium.All       | Chrome web engine apps                                     |
| Linux      | Any/All     |                      | Linux.All          |                        | All Linux distributions                                    |

![alt text](tdf.png "Target Device Families" =750x)

In the app package manifest file, you will need to include the appropriate target device family if you like the package contents to be only extracted on specific platforms and devices. If you like the bulid the package in such a way that it is supported on all platform and device types, choose **Platform.All** as the target device family. Similarly, if you like the package to be only supported on iOS devices, choose **Apple.Ios.All**.

## Sample Manifest File(AppxManifest.xml)

```xml
<?xml version="1.0" encoding="utf-8"?>
<Package xmlns="http://schemas.microsoft.com/appx/manifest/foundation/windows10"
         xmlns:uap3="http://schemas.microsoft.com/appx/manifest/uap/windows10/3">

  <Identity Name="awesome.BestAppExtension"
            Publisher="Awesome Publisher"
            Version="1.0.0.0" />

  <Properties>
    <DisplayName>Best App Extension</DisplayName>
    <PublisherDisplayName>Awesome Publisher</PublisherDisplayName>
    <Description>This is an extension package to my app</Description>
    <Logo>Assets\Logo.png</Logo>
  </Properties>

  <Dependencies>
      <TargetDeviceFamily Name="Platform.All" MinVersion="0.0.0.0" MaxVersionTested="0.0.0.0"/>
  </Dependencies>

  <Applications>
    <Application Id="e504fb41-a92a-4526-b101-542f357b7acb">
        <uap:VisualElements
            DisplayName="Best App Extension" 
            BackgroundColor="white"
            Square150x150Logo="images\squareTile-sdk.png" 
            Square44x44Logo="images\smallTile-sdk.png" 
            AppListEntry="none">
        </uap:VisualElements>

        <Extensions>
            <uap3:Extension Category="Windows.appExtension">
                <uap3:AppExtension Name="add-in-contract" Id="add-in" PublicFolder="Public" DisplayName="Sample Add-in" Description="This is a sample add-in">
                    <uap3:Properties>
                        <!-- Free form space -->
                    </uap3:Properties>
                </uap3:AppExtension>
            </uap3:Extension>
        </Extensions>
        
    </Application>
  </Applications>
</Package>
```

## Platform Version 
In the above sample manifest file, along with the platform name, there are also parameters to specify the **MinVersion** and **MaxVersionTested** These parameters are used on Windows 10 platforms. On Windows 10, the package will only be deployed on Windows 10 OS versions greater than the MinVersion. On other non-Windows 10 platforms, the MinVersion and MaxVersionTested parameters are used by the client apps. The client apps can use this information and make the decision on whether to extract the package contents or not. 

If you would like to use the package for all platforms(Platform.All), we recommend that you use the MinVersion and MaxVersionTested parameters to specify the Windows 10 OS Versions where you would you app to work. **MinVersion** and **MaxVersionTested** are required fields in the manifest and they need to conform the quad notation(#.#.#.#). 

If you are only using the MSIX packaging SDK for only non-Windows 10 platforms, you can simply use '0.0.0.0' as the **MinVersion** and **MaxVersionTested** as the versions. 

## How to effectively use the same package on all platforms (Windows 10 and non-Windows 10)

To make the most of the MSIX Packaging SDK, you will need to build the package in a way that will be deployed like an app package on Windows 10 and at the same time supported on other platforms. On Windows 10, you can build the package as an app extension. To find more information about App Extensoins and how they can help make your app extensible - [you will find it here](https://blogs.msdn.microsoft.com/appinstaller/2017/05/01/introduction-to-app-extensions/). 

In the above manifest file, you will notice within the AppExtension element, there is a section for Properties. There is no validation performed in section of the manifest file. This allows developers to specify the required metadata between extension and host/client app. 

If you have any questions or comments, you can send them [our team](mailto:MSIXPackagingOSSCustomerQs@service.microsoft.com) directly! 
