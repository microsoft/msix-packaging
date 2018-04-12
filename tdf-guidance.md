
# Developer Guidance to target MSIX Packaging SDK

## Build a package to target the SDK 

The MSIX Packaging SDK offers developers a universal way to distribute package contents to client devices regardless of the OS platform on the client device. This enables developers to package their app content once instead of having to package for each platform. 

To take advantage of the new SDK and the ability to distribute your package contents to multiple platforms, we are providing a way to specify the target platforms where you want your packages to extract to. This means, the app developers can ensure that the package contents are being extracted from the package only as desired. 

The following table shows the target device families to declare in the manifest:


<style type="text/css">
.tg  {border-collapse:collapse;border-spacing:0;border-color:#ccc;}
.tg td{font-family:Arial, sans-serif;font-size:14px;padding:10px 5px;border-style:solid;border-width:1px;overflow:hidden;word-break:normal;border-color:#ccc;color:#333;background-color:#fff;}
.tg th{font-family:Arial, sans-serif;font-size:14px;font-weight:normal;padding:10px 5px;border-style:solid;border-width:1px;overflow:hidden;word-break:normal;border-color:#ccc;color:#333;background-color:#f0f0f0;}
.tg .tg-baqh{text-align:center;vertical-align:top}
.tg .tg-yw4l{vertical-align:top}
</style>
<table class="tg">
  <tr>
    <th class="tg-yw4l">Platform</th>
    <th class="tg-yw4l">Family</th>
    <th class="tg-yw4l" colspan="3">Target Device Family</th>
    <th class="tg-yw4l">Notes</th>
  </tr>
  <tr>
    <td class="tg-yw4l" rowspan="6">Windows 10</td>
    <td class="tg-yw4l">Phone</td>
    <td class="tg-031e" rowspan="24"><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br>Platform.All<br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br></td>
    <td class="tg-baqh" rowspan="6">Windows.Universal</td>
    <td class="tg-yw4l">Windows.Mobile</td>
    <td class="tg-yw4l">Mobile devices</td>
  </tr>
  <tr>
    <td class="tg-yw4l">Desktop</td>
    <td class="tg-yw4l">Windows.Desktop</td>
    <td class="tg-yw4l">PC</td>
  </tr>
  <tr>
    <td class="tg-yw4l">Xbox</td>
    <td class="tg-yw4l">Windows.Xbox</td>
    <td class="tg-yw4l">Xbox console</td>
  </tr>
  <tr>
    <td class="tg-yw4l">Surface Hub</td>
    <td class="tg-yw4l">Windows.Team</td>
    <td class="tg-yw4l">Large screen Win 10 devices</td>
  </tr>
  <tr>
    <td class="tg-yw4l">HoloLens</td>
    <td class="tg-yw4l">Windows.Holographic</td>
    <td class="tg-yw4l">VR/AR headset</td>
  </tr>
  <tr>
    <td class="tg-yw4l">IoT</td>
    <td class="tg-yw4l">Windows.IoT</td>
    <td class="tg-yw4l">IoT devices</td>
  </tr>
  <tr>
    <td class="tg-yw4l" rowspan="4">iOS</td>
    <td class="tg-yw4l">Phone</td>
    <td class="tg-yw4l" rowspan="4">Apple.Ios.All</td>
    <td class="tg-yw4l">Apple.Ios.Phone</td>
    <td class="tg-yw4l">iPhone, Touch</td>
  </tr>
  <tr>
    <td class="tg-yw4l">Tablet</td>
    <td class="tg-yw4l">Apple.Ios.Tablet</td>
    <td class="tg-yw4l">iPad mini, iPad, iPad Pro</td>
  </tr>
  <tr>
    <td class="tg-yw4l">TV</td>
    <td class="tg-yw4l">Apple.Ios.TV</td>
    <td class="tg-yw4l">Apple TV</td>
  </tr>
  <tr>
    <td class="tg-yw4l">Watch</td>
    <td class="tg-yw4l">Apple.Ios.Watch</td>
    <td class="tg-yw4l">iWatch</td>
  </tr>
  <tr>
    <td class="tg-yw4l">MacOS</td>
    <td class="tg-yw4l">Desktop</td>
    <td class="tg-baqh" colspan="2">Apple.MacOS.All</td>
    <td class="tg-yw4l">MacBook Pro, MacBook Air, Mac Mini, iMac</td>
  </tr>
  <tr>
    <td class="tg-yw4l" rowspan="5">Android</td>
    <td class="tg-yw4l">Phone</td>
    <td class="tg-yw4l" rowspan="5">Google.Android.All</td>
    <td class="tg-yw4l">Google.Android.Phone</td>
    <td class="tg-yw4l">Mobile devices that target any flavor of Android</td>
  </tr>
  <tr>
    <td class="tg-yw4l">Tablet</td>
    <td class="tg-yw4l">Google.Android.Tablet</td>
    <td class="tg-yw4l">Android tablets</td>
  </tr>
  <tr>
    <td class="tg-yw4l">Desktop</td>
    <td class="tg-yw4l">Google.Android.Desktop</td>
    <td class="tg-yw4l">Chromebooks</td>
  </tr>
  <tr>
    <td class="tg-yw4l">TV</td>
    <td class="tg-yw4l">Google.Android.TV</td>
    <td class="tg-yw4l">Android large screen devices</td>
  </tr>
  <tr>
    <td class="tg-yw4l">Watch</td>
    <td class="tg-yw4l">Google.Android.Watch</td>
    <td class="tg-yw4l">Google gear devices</td>
  </tr>
  <tr>
    <td class="tg-yw4l" rowspan="2">Windows</td>
    <td class="tg-yw4l">7</td>
    <td class="tg-baqh" colspan="2">Windows7.Desktop</td>
    <td class="tg-yw4l">Windows 7 devices</td>
  </tr>
  <tr>
    <td class="tg-yw4l">8</td>
    <td class="tg-baqh" colspan="2">Windows8.Desktop</td>
    <td class="tg-yw4l">Windows 8/8.1 devices</td>
  </tr>
  <tr>
    <td class="tg-yw4l" rowspan="5">Web</td>
    <td class="tg-yw4l">Microsoft</td>
    <td class="tg-yw4l" rowspan="5">Web.All</td>
    <td class="tg-yw4l">Web.Edge.All</td>
    <td class="tg-yw4l">Edge web engine apps</td>
  </tr>
  <tr>
    <td class="tg-yw4l">Android</td>
    <td class="tg-yw4l">Web.Blink.All</td>
    <td class="tg-yw4l">Blink web engine apps</td>
  </tr>
    <tr>
    <td class="tg-yw4l">Chrome</td>
    <td class="tg-yw4l">Web.Chromium.All</td>
    <td class="tg-yw4l">Chrome web engine apps</td>
  </tr>
  <tr>
    <td class="tg-yw4l">iOS</td>
    <td class="tg-yw4l">Web.Webkit.All</td>
    <td class="tg-yw4l">Webkit web engine apps</td>
  </tr>
  <tr>
    <td class="tg-yw4l">MacOS</td>
    <td class="tg-yw4l">Web.Safari.All</td>
    <td class="tg-yw4l">Safari web engine apps</td>
  </tr>
  <tr>
    <td class="tg-yw4l">Linux</td>
    <td class="tg-yw4l">Any/All</td>
    <td class="tg-baqh" colspan="2">Linux.All</td>
    <td class="tg-yw4l">All Linux distributions</td>
  </tr>
</table>

In the app package manifest file, you will need to include the appropriate target device family if you like the package contents to be only extracted on specific platforms and devices. If you like the bulid the package in such a way that it is supported on all platform and device types, choose **Platform.All** as the target device family. Similarly, if you like the package to be only supported in web apps, choose **Web.All**.

## Sample Manifest File (AppxManifest.xml)

```xml
<?xml version="1.0" encoding="utf-8"?>
<Package xmlns="http://schemas.microsoft.com/appx/manifest/foundation/windows10"
         xmlns:mp="http://schemas.microsoft.com/appx/2014/phone/manifest"
         xmlns:uap="http://schemas.microsoft.com/appx/manifest/uap/windows10"
         xmlns:uap3="http://schemas.microsoft.com/appx/manifest/uap/windows10/3"
         IgnorableNamespaces="mp uap uap3">

  <Identity Name="BestAppExtension"
            Publisher="CN=awesomepublisher"
            Version="1.0.0.0" />

  <mp:PhoneIdentity PhoneProductId="56a6ecda-c215-4864-b097-447edd1f49fe" PhonePublisherId="00000000-0000-0000-0000-000000000000"/>

  <Properties>
    <DisplayName>Best App Extension</DisplayName>
    <PublisherDisplayName>Awesome Publisher</PublisherDisplayName>
    <Description>This is an extension package to my app</Description>
    <Logo>Assets\StoreLogo.png</Logo>
  </Properties>
  
  <Resources>
    <Resource Language="x-generate"/>
  </Resources>
  
  <Dependencies>
    <TargetDeviceFamily Name="Platform.All" MinVersion="0.0.0.0" MaxVersionTested="0.0.0.0"/>
  </Dependencies>

  <Applications>
    <Application Id="App">
      <uap:VisualElements
          DisplayName="Best App Extension"
          Description="This is the best app extension"
          BackgroundColor="white"
          Square150x150Logo="images\squareTile-sdk.png"
          Square44x44Logo="images\smallTile-sdk.png"
          AppListEntry="none">
      </uap:VisualElements>

      <Extensions>
        <uap3:Extension Category="Windows.appExtension">
          <uap3:AppExtension Name="add-in-contract" Id="add-in" PublicFolder="Public" DisplayName="Sample Add-in" Description="This is a sample add-in">
            <uap3:Properties>
               <!--Free form space--> 
            </uap3:Properties>
          </uap3:AppExtension>
        </uap3:Extension>
      </Extensions>

    </Application>
  </Applications>
</Package>
```

## Platform Version 
In the above sample manifest file, along with the platform name, there are also parameters to specify the **MinVersion** and **MaxVersionTested** These parameters are used on Windows 10 platforms. On Windows 10, the package will only be deployed on Windows 10 OS versions greater than the MinVersion. On other non-Windows 10 platforms, the MinVersion and MaxVersionTested parameters aren't used to make the declaration of whether to extract the package contents. 

If you would like to use the package for all platforms(Windows 10 and non-Windows 10), we recommend that you use the MinVersion and MaxVersionTested parameters to specify the Windows 10 OS Versions where you would like your app to work. So your manifest's **Dependencies** section would look like this:
```xml
  <Dependencies>
    <TargetDeviceFamily Name="Platform.All" MinVersion="0.0.0.0" MaxVersionTested="0.0.0.0"/>
    <TargetDeviceFamily Name="Windows.Universal" MinVersion="10.0.14393.0" MaxVersionTested="10.0.16294.0"/>
  </Dependencies>
```

**MinVersion** and **MaxVersionTested** are required fields in the manifest and they need to conform the quad notation(#.#.#.#). If you are only using the MSIX packaging SDK for only non-Windows 10 platforms, you can simply use '0.0.0.0' as the **MinVersion** and **MaxVersionTested** as the versions. 

## How to effectively use the same package on all platforms (Windows 10 and non-Windows 10)

To make the most of the MSIX Packaging SDK, you will need to build the package in a way that will be deployed like an app package on Windows 10 and at the same time supported on other platforms. On Windows 10, you can build the package as an app extension. To find more information about App Extensions and how they can help make your app extensible - [you will find it here](https://blogs.msdn.microsoft.com/appinstaller/2017/05/01/introduction-to-app-extensions/). 

In the above manifest file, you will notice within the AppExtension element, there is a section for Properties. There is no validation performed in this section of the manifest file. This allows developers to specify the required metadata between extension and host/client app. 

If you have any questions or comments, you can send them [our team](mailto:MSIXPackagingOSSCustomerQs@service.microsoft.com) directly! 
