
# Location forms

When editing locations in the Creation Kit, you'll see some of the properties you'd expect: a name, some configurable markers, and whatnot. However, locations are an iceberg: there's a lot of data below the surface, such as their content lists.

## Content lists

Location forms contain six lists defining different kinds of content within the location, whose subrecords take the form `LC__`, `AC__`, and `RC__`:

* The base record[^base-record] uses `L` to define the initial entries in the list.
* Override records use `A` to define **a**dditions to the list, or modifications to its entries.
* Override records use `R` to define **r**emovals from the `L` entries.

[^base-record]: The base record is the LCTN record that originally defines/creates the location form, as opposed to overrides within later files.

These lists don't follow the Rule of One. That is: overrides don't clear (and potentially replace) the list contents defined in previously-loaded overrides or the base records. This is why `A`-subrecords and `R`-subrecords for these lists are able to exist. (All other location data does follow the Rule of One.)

The six content lists are:

| Subrecord | List | Description |
| :- | :- | :- |
| `_CPR` | **P**ersist Location **R**efs | All non-deleted ObjectReferences whose Persist Location is this Location. |
| `_CUN` | **Un**ique Actors | All non-deleted Actors whose ActorBases are flagged Unique, and whose Persist Location is this Location. |
| `_CSR` | **S**pecial **R**efs | All ObjectReferences that have a LocRefType and are direct children of a Cell belonging[^cell-belongs] to this Location. |
| `_CEC` | **E**xterior **C**ells | Entries in a key/value map, listing worldspaces and their child cells that belong[^cell-belongs] to this Location. There is one `_CEC` subrecord per relevant worldspace. |
| `_CID` | **I**nitially **D**isabled Refs | All non-deleted ObjectReferences whose Persist Location is this Location, and who are flagged as Initially Disabled. This list doesn't support removals; there is no `RCID` subrecord. |
| `_CEP` | **E**nable **P**arentage | All non-deleted ObjectReferences whose Persist Location is this Location, and who have an Enable State Parent. This list doesn't support removals; there is no `RCEP` subrecord. |

[^cell-belongs]: The location that a cell belongs to is the location that the following (in order of descending priority) identify themselves as belonging to: the cell's encounter zone; the parent world's encounter zone; the cell itself; or the cell's parent world. For example, even if a cell identifies itself as belonging to location *A*, if it also identifies itself as belonging to an encounter zone whose own location is *B*, then the cell's computed location is *B*.

The Creation Kit itself doesn't load all of these subrecords, and when it saves the subrecords out, it generates them from scratch rather than relying on the originally loaded data. Specifically, the CK searches the location's inbound Use Info to find any forms that may belong in the lists above, and then double-checks each form and regenerates the lists accordingly.

When saving an override record, it only produces `AC__` entries when those entries aren't identical to any corresponding `LC__` entries; and it produces `RC__` entries when the entity in question (the ref or exterior cell) exists in the loaded `LC__` but no longer belongs in the relevant content list. A few examples, when saving a location override:

* If a ref is listed in `LCPR` and then is moved to another cell by an override, then it'll get an `ACPR` entry. The `_CPR` entries include the containing interior cell or worldspace and grid coords, so the entry for that ref will be changed.
* If a ref is listed in `LCPR` and isn't modified in any way that would alter its `_CPR` data, then it won't get an `ACPR` entry; it isn't modified.
* If a ref is listed in `LCPR` but is moved entirely outside of this location, then it'll get an `RCPR` entry.

There are some quirks within the Creation Kit's behavior:

* `LCPR` isn't loaded initially. At save time, the CK will load `LCPR` subrecords from the location's first source file only (i.e. the base record), and basically diff them against the refs it plans on saving into `ACPR` in order to then generate `RCPR`.
* Other `L`-subrecords actually are loaded, and are what the save process diffs against. Among other things, this means that `L`-subrecords that are incorrectly placed within an override record will be treated as if they originate from the base record.

### DovahKit

As of this writing, DovahKit doesn't attempt to fully emulate the above-listed quirks nor their effects on misplaced `A`-/`L`-subrecords. An `L`-subrecord in an override will trigger an editor warning, and will be treated as if it were an `A`-subrecord. Similarly, an `A`-subrecord in a base record will trigger a warning and will be treated as though it were an `L`-subrecord.

DovahKit maintains two versions of each content list:

* The `base` list consists only of `LC__` (and misplaced `AC__`) subrecord content from the base record.
* The `full` list consists of the "final" list: `LC__` plus `AC__` minus `RC__`.

DovahKit tracks use info for both versions of each list. If a ref is in both `LC__` and `AC__`, and isn't removed via `RC__`, then that counts as two outbound uses to the ref.

When saving, DovahKit wholly regenerates the lists (similarly to the Creation Kit) using `dovah::utils::update_location_content`. This is invoked by `Location::save`.
