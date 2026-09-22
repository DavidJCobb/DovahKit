;
; Ensure that the VC version variables in `vc-version.iss` are up to date. Visual Studio 
; will bundle the VC Redist installer when compiling; code in our MSBuild project will 
; rummage through your VS install to see what VC Redist version is being bundled, and 
; update the `vc-version.iss` file.
;
#include "vc-version.iss"

#define VCInstallMessage "Installing Microsoft Visual C++ Redistributable...."

[Run]
Filename: "{app}\vc_redist.x64.exe"; Parameters: "/install /passive"; StatusMsg: "{#VCInstallMessage}"; Check: IsWin64 and not VCInstalled

[Code]
function VCInstalled: Boolean;
var
  major: Cardinal;
  minor: Cardinal;
  bld:   Cardinal;
  rbld:  Cardinal;
  key:   String;
begin
  Result := False;
  key    := 'SOFTWARE\Microsoft\VisualStudio\14.0\VC\Runtimes\x64';
  if RegQueryDWordValue(HKEY_LOCAL_MACHINE, key, 'Major', major) then begin
    if RegQueryDWordValue(HKEY_LOCAL_MACHINE, key, 'Minor', minor) then begin
      if RegQueryDWordValue(HKEY_LOCAL_MACHINE, key, 'Bld', bld) then begin
        if RegQueryDWordValue(HKEY_LOCAL_MACHINE, key, 'RBld', rbld) then begin
            Log('Existing VC redist version: ' + IntToStr(major) + '.' + IntToStr(minor) + '.' + IntToStr(bld) + '.' + IntToStr(rbld));
            Result := (major >= {#vc_version_major}) and (minor >= {#vc_version_minor}) and (bld >= {#vc_version_bld}) and (rbld >= {#vc_version_rbld})
        end;
      end;
    end;
  end;
end;
