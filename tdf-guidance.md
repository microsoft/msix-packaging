
#MSIX Packaging SDK
   Copyright (c) 2017 Microsoft Corp.  All rights reserved.

## Build a package to target the SDK 

The MSIX Packaging SDK offers developers a universal way to distribute package contents to client devices regardless of the OS platform on the client device. This enables developers to package their app content once instead of having to package for each platform. 

To take advantage of the new SDK and the ability to distribute your package contents to multiple platforms, we are providing a way to specify the target platforms where you want your packages to extract to. This means, the app developers can ensure that the package contents are being extracted from the package only as desired. 

The following table shows the target device families to declare in the manifest:

![alt text](tdf.png "Target Device Families" =750x)

In the app package manifest file, you will need to include the appropriate target device family if you like the package contents to be only extracted on specific platforms and devices. If you like the bulid the package in such a way that it is supported on all platform and device types, choose "Platform.All" as the target device family. Similarly, if you like the package to be only supported on iOS devices, choose "Apple.Ios.All".
## Sample Manifest File(AppxManifest.xml)

```xml
<?xml version="1.0" encoding="utf-8"?>
<Package xmlns="http://schemas.microsoft.com/appx/manifest/foundation/windows10"
         xmlns:uap3="http://schemas.microsoft.com/appx/manifest/uap/windows10/3">

  <Identity Name="awesome.BestApp"
            Publisher="Awesome Publisher"
            Version="1.0.0.0" />

    <Properties>
    <DisplayName>Best App</DisplayName>
    <PublisherDisplayName>Awesome Publisher</PublisherDisplayName>
    <Description>msix package</Description>
    <Logo>Assets\Logo.png</Logo>
  </Properties>

  <Dependencies>
      <TargetDeviceFamily Name="Platform.All" MinVersion="0.0.0.0" MaxVersionTested="0.0.0.0"/>
  </Dependencies>

  <Applications>
    <Application Id="e504fb41-a92a-4526-b101-542f357b7acb">
        <uap:VisualElements
            DisplayName="Best App" 
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



