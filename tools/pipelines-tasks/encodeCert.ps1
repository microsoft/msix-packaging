<#
.SYNOPSIS
    This script imports the pfx certificate and converts it to a base64 utf-8 encoded string

.DESCRIPTION
    This script exports the pfx certificate and converts it to a base64 utf-8 encoded string

.PARAMETER filePath
    Specifies the file location of the pfx certificate 
#>

param(
    [string]$filePath
)

#Convert Password to Secure String
$password = New-Object SecureString
foreach ($char in "password".ToCharArray()) {
    $password.AppendChar($char)
}

#Set params to import certificate
$params = @{
    FilePath = $filePath
    CertStoreLocation = "Cert:\CurrentUser\My"
    Password = $password
}

#Import Certificate and convert to base 64 string
$cert = Import-PfxCertificate @params
$certBase64 = [System.Convert]::ToBase64String($cert.Export([System.Security.Cryptography.X509Certificates.X509ContentType]::Cert))
$certUtf8 = [System.Text.Encoding]::UTF8.GetBytes($certBase64)

#Output
$certUtf8