
## Add-ons

One of the things that `TESModel` can pre-cache is a list of add-on node IDs. But what the hell do an of those words mean?

Bethesda built a system called **add-ons**, which basically allows one NIF to include the contents of another NIF by reference. You can't specify arbitrary NIF files; rather, the game data defines a library of reusable NIFs.

* In your ESP file, you can define an Add-on form (`ADDN` i.e. `form_type::addon_node`) which consists of a 3D model and a unique numeric ID.
* In any NIF file, you can create a BSValueNode. This is, functionally, just a local transform (i.e. offset from its parent node) and a value. In practice, however, the "value" specifies one of those unique numeric IDs. Skyrim will look up the appropriate `ADDN`, load its NIF, and copy the contents of that NIF to where the BSValueNode is.

Here are a few examples:

* `Effects\CWFXCatapultProjectile.nif` uses several add-on nodes with IDs 5, 6, 71, and 74.
  * The ID 5 is defined by `[ADDN:0001CBB2]MPSFireBallSparkTrail`.
  * The ID 6 is defined by `[ADDN:0001CBB1]MPSFireBallSmokeTrail`.
  * The ID 71 is defined by `[ADDN:000E1532]MPSDragonFireWave`.
  * The ID 74 is defined by `[ADDN:000EE9ED]MPSFireBallSmokeTrailBig`.

  Each of these Add-on Node forms specifies a NIF file; so, a Civil War catapult projectile can include the 3D effects for sparks, smoke, flames, and smoke trails entirely by reference.

* `Effects\AshPile01.nif` uses a single add-on node with ID 61. That ID is defined by `[ADDN:00E7556]MPSAshPile01`, which seems to be purpose-built for this particular model. At this time I don't have the means to easily search and see if anything else uses it.

Which brings us back to the `TESModel` data, in the `MODT` and similar subrecords. This data will list all add-on IDs used by the NIF, so that when the game wants to load the model, it can also go and load those add-on node definitions' NIFs with it.

## Texture swaps

A `TESModelTextureSwap` contains a list of texture swaps (`::TEX_SWAP` in-game) to apply to a NIF tree. The structure is as follows:

```c++
struct TEX_SWAP {
   BGSTextureSet* texture_set = nullptr;
   int            nif_leaf_index; // uint32_t in practice
   BSFixedString  nif_block_name; // just pretend it's a const char*
};
```

Note that the `leaf_index` is *not* the same as the block indices seen in the NIF file or in NifSkope. Rather, to compute the "leaf index" of any given `NiGeometry` block, use this algorithm:

```c++
int leaf_index_of(NiNode* root_node, NiGeometry* desired) {
   constexpr const int not_found = -1;

   int current_leaf_index = 0;
   return [&current_leaf_index, desired](this auto&& recurse, NiAVObject* current_block) -> int {
      if (!current_block)
         return not_found;
      if (current_block == desired)
         return current_leaf_index;

      if (auto* node = dynamic_cast<NiNode*>(current_block)) {
         for(auto* child : node->children) {
            auto n = recurse(child);
            if (n != not_found)
               return n;
         }
         return not_found;
      }

      ++current_leaf_index;

      return not_found;
   }(root_node);
}
```

### Matching a texture swap to a geometry block

The Creation Kit relies on the block name: when loading a `TESModelTextureSwap`, it checks the target NIF file and for each `TEX_SWAP`, it looks for a block with a matching name. If no match is found (bearing in mind that nameless swaps always fail to match), then the swap is stripped out. Otherwise, the swap's index is updated to point to the found block. As far as I know, this happens on load, which in turn would imply that the CK is mass-loading and mass-unloading every referenced NIF on startup. (Oof.)

* See: 32-bit Creation Kit subroutine at offset 55F850, which I call `TESModelTextureSwap::InitItemImpl`. It's invoked by overrides for the virtual function `TESForm::InitItemImpl`.
* See: 32-bit Creation Kit subroutine at offset 55F0B0, which I call `TESModelTextureSwap::UpdateTextureSwapIndices`. It's called by `InitItemImpl` for each `TEX_SWAP`, and is what performs those lookups.

The game itself, however, relies solely on the leaf index.

* See: 32-bit Skyrim ("LE") subroutine at offset 4557B0, which I call `TESModelTextureSwap::ApplyToNodeTree(NiNode*)`. This subroutine constructs a disposable map, fills it with block indices and texture swap pointers, and then passes it to an underlying recursive function which is used to actually carry out the swap process.
* See: 32-bit Skyrim ("LE") subroutine at offset 455660, which I call `TESModelTextureSwap::ApplyToNodeTree_Impl(NiAVObject*, BSScrapMap<int, TEX_SWAP*>&, int& current_leaf_index)`. (The caller should pass zero as the third argument.) This subroutine recurses over a given NIF tree, applying texture swaps as `NiGeometry` blocks with matching leaf indices are found.

### Special cases

When you use the hardcoded `NullTextureSet` form as a texture swap, the game handles it differently. The game doesn't modify any textures on the target `NiGeometry` block; instead, it sets the target geometry block's "culled" flag, forcibly hiding it.

If a `NiGeometry` has no `BSShaderTextureSet` but is the target of a texture swap, then it will have a `BSShaderTextureSet` created for it at run-time. (Actually, the game *always* creates a new `BSShaderTextureSet` from the `BGSTextureSet` you're applying, and just discards any that already existed.)