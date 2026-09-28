#if 0
   ;
   ; For use in a [Files] section. Checks if the specified directory name exists within $(ProjectDir); 
   ; if so, sets that directory up to be copied into the installer and eventually unpacked into the 
   ; installed app directory.
   ;
#endif
#define IncludeSrcDirIfExists(Str dir, Int allow_user_modify = 0, Str exclusions = "") \
   DirExists(CppProjectPath + dir) ? \
      ( \
         ('Source: "' + CppProjectPath + dir + '\*"; DestDir: "{app}\' + dir + '\"; Flags: ignoreversion recursesubdirs createallsubdirs') + \
         (allow_user_modify != 0 ? '; Permissions: users-modify' : '') + \
         (exclusions ? '; Excludes: "' + exclusions + '"' : '') \
      ) \
   : \
      ""