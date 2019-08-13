using System;
using System.Collections.Generic;
using System.Text;
using Microsoft.Deployment.WindowsInstaller;
using Microsoft.Win32;

namespace GetMsixmgrProducts
{
    public class CustomActions
    {
        [CustomAction]
        public static ActionResult GetMsixmgrProducts(Session session)
        {
            session.Log("Begin GetMsixmgrProducts");
            
            // Determine all the MSIX packages installed by the msixmgr installed to this location.
            // It could be possible other MSIX packages are installed using msixmgrLib or clickonce msixmgr.
            // So, we check the Uninstall key specifically for the install location this uninstall would uninstall
            // instead of using msixmgr to enumerate products
            String msixmgrInstalledProducts = "";
            session.Log(session["INSTALLFOLDER"]);

            using (RegistryKey hklm64 = RegistryKey.OpenBaseKey(RegistryHive.LocalMachine, RegistryView.Registry64))
            {
                using (RegistryKey uninstallKey = hklm64.OpenSubKey(@"SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall", false))
                {
                    if (uninstallKey != null)
                    {
                        foreach (String uninstallKeyName in uninstallKey.GetSubKeyNames())
                        {
                            session.Log("Uninstallkeyname " + uninstallKeyName);
                            using (RegistryKey productKey = uninstallKey.OpenSubKey(uninstallKeyName, false))
                            {
                                if (productKey != null)
                                {
                                    String uninstallString = (String)productKey.GetValue("UninstallString", "");

                                    if (uninstallString.Length > 0)
                                    {
                                        session.Log("UninstallString " + uninstallString);
                                        if (uninstallString.Contains(session["INSTALLFOLDER"]))
                                        {
                                            // found a product, add the displayName to our string to return
                                            String displayName = (String)productKey.GetValue("DisplayName", uninstallKeyName);

                                            if (msixmgrInstalledProducts.Length > 0)
                                            {
                                                msixmgrInstalledProducts += " ";
                                            }
                                            msixmgrInstalledProducts += displayName;
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
            
            session.Log("Products found: " + msixmgrInstalledProducts);
            if (msixmgrInstalledProducts.Length > 0)
            {
                session["MSIXMGR_PRODUCTS"] = msixmgrInstalledProducts;
            }
            else
            {
                session["WixUI_InstallMode"] = "Remove";
            }
            
            return ActionResult.Success;
        }
    }
}
