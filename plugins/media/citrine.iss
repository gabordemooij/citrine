[Setup]
AppId=ctrnl
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
DefaultGroupName=CitrineNL
UninstallDisplayIcon={app}\ctrnl.exe
Compression=none
SolidCompression=yes
WizardImageFile=citrine_install_banner.bmp
WizardSmallImageFile=install_mini.bmp
LicenseFile=license.txt
SetupIconFile=setup.ico
ShowLanguageDialog=yes


[Dirs]
Name: "{app}\mods"
Name: "{app}\mods\media"
Name: "{app}"; Permissions: users-full


[Files]
Source: "dist\*"; DestDir: "{app}"
Source: "dist\mods\media\*"; DestDir: "{app}\mods\media\"


[Icons]
Name: "{commonprograms}\CitrineNL"; Filename: "{app}\ctrnl.exe"; Parameters: "__7__"; WorkingDir: "{app}"; IconFilename: "{app}\pictogram.ico"

[Languages]
Name: "en"; MessagesFile: "compiler:Default.isl"
Name: "nl"; MessagesFile: "compiler:Languages\Dutch.isl"





