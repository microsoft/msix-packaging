using System;
using System.Windows.Forms;
using System.Deployment.Application;
using System.Collections.Specialized;
using System.Web;

namespace ClickOnceWrapper
{
    static class ClickOnceWrapper
    {
        static bool IsRS3OrAbove()
        {
            Version osVersion = Environment.OSVersion.Version;
            if (osVersion.Major >= 10 &&
                osVersion.Build >= 16299)
            {
                return true;
            }
            else
            {
                return false;
            }
        }
        /// <summary>
        /// The functionality of this ClickOnce Wrapper is to wrap the msixmgr.exe and msix.dll installer binaries.
        /// This ClickOnce app is intended to be hosted on a website, and be passed in a URL to an .msix package
        /// This app will then install that .msix package.
        /// </summary>
        [STAThread]
        static void Main()
        {
            NameValueCollection col = new NameValueCollection();

            if (ApplicationDeployment.IsNetworkDeployed && ApplicationDeployment.CurrentDeployment.ActivationUri != null)
            {
                string queryString = ApplicationDeployment.CurrentDeployment.ActivationUri.Query;
                col = HttpUtility.ParseQueryString(queryString);

                string decode = HttpUtility.UrlDecode(col.ToString());

                System.Diagnostics.ProcessStartInfo processInfo;
                if (IsRS3OrAbove())
                {
                    // re-route directly to desktop app installer
                    string desktopAppInstallerProtocol = "ms-appinstaller:?source=" + decode;
                    processInfo = new System.Diagnostics.ProcessStartInfo(desktopAppInstallerProtocol);
                }
                else
                {
                    string arguments = "-addpackage " + decode;
                    // It is not possible to launch a ClickOnce app as administrator directly,
                    // so instead we launch the app as administrator in a new process.
                    processInfo = new System.Diagnostics.ProcessStartInfo("msixmgr.exe", arguments);

                    // The following properties run the new process as administrator
                    processInfo.UseShellExecute = true;
                    processInfo.Verb = "runas";
                }

                // Start the new process
                try
                {
                    System.Diagnostics.Process.Start(processInfo);
                }
                catch (Exception)
                {
                    MessageBox.Show("Unable to start this program");
                }
                
                // Shut down the current process
                Application.Exit();
            }
            else
            {
                MessageBox.Show("Unexpected launch, this clickonce app is expected to launch from http URL with extra URL arguments");
            }
        }
    }
}
