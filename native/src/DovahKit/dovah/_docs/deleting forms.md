
# Deleting forms

Forms can be "deleted" in two ways:

* Forms in the active file will have their `form_stub`s wholly deleted from memory.

* Forms not in the active file will merely be flagged as "deleted." This is a record header flag.

## Notes

### The game engine

* Many form types have loaders, in-game, that skip loading any data if the form is flagged as "deleted."

* I assume that the "deleted" record flag exists for the sake of Bethesda's version control, because it's not especially stable in-game:

  * Refs flagged as deleted may appear at (0, 0, 0) in their containing worldspace. This is because the REFR loader skips everything in a deleted record, including the DATA subrecord which contains the ref's coordinates.
  
  * Some sources online claim that refs flagged as deleted can cause crashes if another mod tries to override them. I have not verified this.
  
  * Some sources online claim that deleted navmeshes can crash the game when their parent cell is loaded, if those navmeshes are used by other forms or if the records are overridden by another mod. I have not verified this.
  
  * Any override of a "deleted" record that is not, itself, "deleted" will have the effect of un-deleting the record.
  
### DovahKit

* DovahKit creates dummy `form_stub`s to represent form IDs taken up by game settings. I don't recall offhand whetehr DovahKit specifically prevents frontends from deleting these stubs. Attempting to do so should be considered undefined behavior. (Off the top of my head, I think it'd result in other forms or game settings potentially being able to share the same form ID by accident. Trying to re-number the game setting could then lead to further undefined behavior, such as unexpected operations on whatever form stub is sharing that ID.)
