using System;
using System.Collections.Generic;
using System.Text;
using Microsoft.Deployment.WindowsInstaller;
using Microsoft.Win32;

namespace CustomAction1
{
    public class CustomActions
    {
        [CustomAction]
        public static ActionResult CustomAction1(Session session)
        {
            session.Log("Begin CustomAction1");
            
            // Determine all the MSIX packages installed by msixmgr. 
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
                                            // found a product, add the displayName to our string
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
            
            session.Log("After " + msixmgrInstalledProducts);
            session["MSIXMGR_PRODUCTS"] = msixmgrInstalledProducts;
            return ActionResult.Success;
        }
    }
}
