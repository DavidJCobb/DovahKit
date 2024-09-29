
Most data about a form is only retained when the form is fully loaded. There are a lot of situations where there's some data that the backend doesn't need to load (read: it's not worth moving to `form_stub_addenda`) but the frontend needs to be able to access it quickly &mdash; and ideally without the overhead of fully loading the form. In these cases, we use the Form Info Cache subsystem to keep that data cached.

Current list:

* Quest filter strings (the Object Window nesting options)
* Model paths
* List of all DIALs that can legally contain SharedInfos
* VMAD info
  * Needed to help with the Papyrus subsystem
* General form info
  * Faction (FACT) info
    * Tracks crime?
  * HeadPart (HDPT) info
    * Is extra?
    * Is playable?
    * Race list
    * Sex
    * Type
  * Voicetype (VTYP) info
    * Is female?