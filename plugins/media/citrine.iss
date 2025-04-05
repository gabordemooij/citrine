[Setup]
AppId=ctren
ArchitecturesInstallIn64BitMode=x64
DisableWelcomePage=no
DisableDirPage=yes
DisableProgramGroupPage=yes
SourceDir=C:\InnoSetupSourceDir
OutputBaseFilename=Citrine{VERSION}
AppName=CitrineNL
AppVersion={VERSION_NAME}
AppCopyright=Copyright (C) 2009-2025 GaborSoftware
WizardStyle=modern
DefaultDirName={autopf}\CitrineNL
DefaultGroupName=CitrineEN
UninstallDisplayIcon={app}\ctren.exe
Compression=none
SolidCompression=yes
WizardImageFile=citrine_install_banner.bmp
WizardSmallImageFile=install_mini.bmp
LicenseFile=license.txt
SetupIconFile=setup.ico
ShowLanguageDialog=yes


[Dirs]
Name: "{app}\mods"
Name: "{app}\mods\gui"
Name: "{app}"; Permissions: users-full


[Files]
Source: "dist\*"; DestDir: "{app}"
Source: "dist\mods\gui\*"; DestDir: "{app}\mods\gui\"


[Languages]
Name: "en"; MessagesFile: "compiler:Default.isl"





