; �ű��� Inno Setup �ű��� ���ɣ�
; �йش��� Inno Setup �ű��ļ�����ϸ��������İ����ĵ���

#define MyAppName "��ɳ����ѧ��������Ϣ����ѧԺ�Ծ��Զ�����ϵͳ"
#define MyAppVersion "1.0"
#define MyAppPublisher "��ɳ����ѧ��������Ϣ����ѧԺ"
#define MyAppURL "http://www.csust.edu.cn"
#define MyAppExeName "QtActive.exe"

[Setup]
; ע: AppId��ֵΪ������ʶ��Ӧ�ó���
; ��ҪΪ������װ����ʹ����ͬ��AppIdֵ��
; (�����µ�GUID����� ����|��IDE������GUID��)
AppId={{8350400D-4947-47CA-AB34-D5403AA34DCE}
AppName={#MyAppName}
AppVersion={#MyAppVersion}
;AppVerName={#MyAppName} {#MyAppVersion}
AppPublisher={#MyAppPublisher}
AppPublisherURL={#MyAppURL}
AppSupportURL={#MyAppURL}
AppUpdatesURL={#MyAppURL}
DefaultDirName={pf}\{#MyAppName}
DefaultGroupName=��������Ϣ����ѧԺ�Ծ��Զ�����ϵͳ
LicenseFile=E:\chouti\licence.txt
InfoBeforeFile=E:\chouti\ReadMe.txt
OutputDir=C:\Documents and Settings\Administrator\����\chouti
OutputBaseFilename=setup
Compression=lzma
SolidCompression=yes

[Languages]
Name: "chinesesimp"; MessagesFile: "compiler:Default.isl"

[Tasks]
Name: "desktopicon"; Description: "{cm:CreateDesktopIcon}"; GroupDescription: "{cm:AdditionalIcons}"; Flags: unchecked; OnlyBelowVersion: 0,6.1

[Files]
Source: "C:\QtWorkspace\QtActive-build-desktop-Qt_4_8_5__4_8_5____\debug\QtActive.exe"; DestDir: "{app}"; Flags: ignoreversion
Source: "C:\QtWorkspace\QtActive-build-desktop-Qt_4_8_5__4_8_5____\debug\QtActive.exe"; DestDir: "{app}"; Flags: ignoreversion
Source: "C:\QtWorkspace\QtActive-build-desktop-Qt_4_7_4_for_Desktop_-_MinGW_4_4__Qt_SDK____\debug\libgcc_s_dw2-1.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "C:\QtWorkspace\QtActive-build-desktop-Qt_4_7_4_for_Desktop_-_MinGW_4_4__Qt_SDK____\debug\mingwm10.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "C:\QtWorkspace\QtActive-build-desktop-Qt_4_7_4_for_Desktop_-_MinGW_4_4__Qt_SDK____\debug\QtCored4.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "C:\QtWorkspace\QtActive-build-desktop-Qt_4_7_4_for_Desktop_-_MinGW_4_4__Qt_SDK____\debug\QtGuid4.dll"; DestDir: "{app}"; Flags: ignoreversion
; ע��: ��Ҫ���κι���ϵͳ�ļ���ʹ�á�Flags: ignoreversion��

[Icons]
Name: "{group}\{#MyAppName}"; Filename: "{app}\{#MyAppExeName}"
Name: "{group}\{cm:UninstallProgram,{#MyAppName}}"; Filename: "{uninstallexe}"
Name: "{commondesktop}\{#MyAppName}"; Filename: "{app}\{#MyAppExeName}"; Tasks: desktopicon

[Run]
Filename: "{app}\{#MyAppExeName}"; Description: "{cm:LaunchProgram,{#StringChange(MyAppName, '&', '&&')}}"; Flags: nowait postinstall skipifsilent

