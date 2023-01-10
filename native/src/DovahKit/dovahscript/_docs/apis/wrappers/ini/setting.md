
# ini_setting

A Lua userdata type representing an INI setting loaded from the game.

## Instance properties

<dl>
   <dt>current_value</dt>
   <dd>
      The current value of the INI setting, as loaded from the INI file. This value will have whatever type is appropriate for the setting (i.e. boolean, integer, number, or string).
   </dd>
   <dt>default_value</dt>
   <dd>
      The default value of the INI setting, as extracted from the game executable. This value will have whatever type is appropriate for the setting (i.e. boolean, integer, number, or string).
   </dd>
   <dt>name</dt>
   <dd>
      The name of the INI setting.
   </dd>
</dl>