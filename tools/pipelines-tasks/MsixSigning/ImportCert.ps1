<#
.Description
Adds or removes a certificate encoded as a string to/from the default cert store.

.Outputs
System.String. The thumbprint of the certificate.
#>

using namespace System.Security.Cryptography.X509Certificates

[CmdletBinding()]
param (
    # Contents of the .pfx certificate file encoded as a base64 string.
    [Parameter()]
    [string]
    $certBase64,

    # Remove the certificate from the cert store instead of adding it.
    [Parameter()]
    [switch]
    $remove
)

# Convert the base64 string to a certificate object.
# We want to persist the private key to the cert store when we import it to be able to sign.
$certBytes = [System.Convert]::FromBase64String($certBase64)
$cert = New-Object -TypeName X509Certificate2 -ArgumentList ($certBytes, $null, [X509KeyStorageFlags]::PersistKeySet) -ErrorAction Stop

# Open the default cert store. This is the current user's "My" (Personal) store.
$certStore = New-Object -TypeName X509Store
$certStore.Open([OpenFlags]::OpenExistingOnly -bor [OpenFlags]::ReadWrite)

# Add or remove the certificate as needed.
if ($remove)
{
    $certStore.Remove($cert)
}
else
{
    $certStore.Add($cert)
}

$certStore.Close()
return $cert.Thumbprint