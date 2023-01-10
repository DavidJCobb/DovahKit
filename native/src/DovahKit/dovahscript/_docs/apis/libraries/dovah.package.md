
# dovah.package

A library available only if a script package is currently running. Script packages can use this to access any files bundled with their code.

## Methods

<dl>
   <dt>dovah.package.load_file(...)</dt>
   <dd>
      Loads a file from the script package, using the same logic and constraints as <code>dovah.load_game_asset(...)</code> except that paths are relative to the script package folder. The use of paths outside of the script package folder is not allowed and will throw an error.
   </dd>
</dl>