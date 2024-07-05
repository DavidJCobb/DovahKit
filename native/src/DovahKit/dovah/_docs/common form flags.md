
The following flags are known to exist on all `TESBoundObject` subclasses. In the Creation Kit, function `TESForm::Unk_0A` (apparently responsible for initializing a form's editing dialog based on the form's own data) sets up fields for these flags based on whether the form is a `TESBoundObject`:

* `TESForm::GetObstacle()` -> "Obstacle"
* `00010000` -> "Random Anim Start" (via vfunc, so a form type could theoretically override this and redirect the flag)
* `00000040` -> control ID 2660 ("Has Tree LOD")
* `00000080` -> control ID 2301
* `00008000` -> control ID 2297 ("Has Distant LOD")
* `00020000` -> control ID 2298 ("Uses High-Detail LOD Texture")
* `10000000` -> control ID 2300

In the Creation Kit only, `TESForm` virtual member function 0x2D returns an enum representing the "Navmesh Generation Import Option" value. The CK calls it blindly on all form types, and uses it to control which radio buttons are checked.