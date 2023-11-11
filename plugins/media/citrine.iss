[Setup]
AppId=ctrnl
ArchitecturesInstallIn64BitMode=x64
DisableWelcomePage=no
DisableDirPage=yes
DisableProgramGroupPage=yes
SourceDir=C:\InnoSetupSourceDir
OutputBaseFilename=Citrine096
AppName=CitrineNL
AppVersion=0.9.6
AppCopyright=Copyright (C) 2009-2024 GaborSoftware
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
Name: "hy"; MessagesFile: "compiler:Languages\Armenian.isl"
Name: "br_pt"; MessagesFile: "compiler:Languages\BrazilianPortuguese.isl"
Name: "bg"; MessagesFile: "compiler:Languages\Bulgarian.isl"
Name: "ca"; MessagesFile: "compiler:Languages\Catalan.isl"
Name: "co"; MessagesFile: "compiler:Languages\Corsican.isl"
Name: "cz"; MessagesFile: "compiler:Languages\Czech.isl"
Name: "da"; MessagesFile: "compiler:Languages\Danish.isl"
Name: "nl"; MessagesFile: "compiler:Languages\Dutch.isl"
Name: "fi"; MessagesFile: "compiler:Languages\Finnish.isl"
Name: "fr"; MessagesFile: "compiler:Languages\French.isl"
Name: "de"; MessagesFile: "compiler:Languages\German.isl"
Name: "hu"; MessagesFile: "compiler:Languages\Hungarian.isl"
Name: "is"; MessagesFile: "compiler:Languages\Icelandic.isl"
Name: "it"; MessagesFile: "compiler:Languages\Italian.isl"
Name: "jp"; MessagesFile: "compiler:Languages\Japanese.isl"
Name: "no"; MessagesFile: "compiler:Languages\Norwegian.isl"
Name: "pl"; MessagesFile: "compiler:Languages\Polish.isl"
Name: "pt"; MessagesFile: "compiler:Languages\Portuguese.isl"
Name: "ru"; MessagesFile: "compiler:Languages\Russian.isl"
Name: "sk"; MessagesFile: "compiler:Languages\Slovak.isl"
Name: "sl"; MessagesFile: "compiler:Languages\Slovenian.isl"
Name: "es"; MessagesFile: "compiler:Languages\Spanish.isl"
Name: "tr"; MessagesFile: "compiler:Languages\Turkish.isl"
Name: "zh"; MessagesFile: "compiler:Languages\Chinese.isl"





