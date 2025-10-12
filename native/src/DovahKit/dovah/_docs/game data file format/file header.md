
# File headers

The file header for a game data file is a record with the `TES4` signature.

## Override list

Non-persistent refs (if defined in master-flagged files) aren't loaded at startup; they're only loaded on demand, when their containing cell is loaded.  The process of loading a cell and its containing refs is like so:

```cpp
// Influenced by TES4/ONAM:
static std::unordered_map<form_id_t, file_t*> winning_record_sources;

for(auto& file : all_loaded_files) {
   if (!file.contains(cell))
      continue;
   auto& grup = file.record_for(cell).child_group(temporary_children);
   for (auto& record : grup) {
      auto  form_id = file.record_id_to_form_id(record.recordID);
      auto* form    = ::lookup_form_by_id(form_id);
      if (form)
         continue;
      auto it = winning_record_sources.find(form_id);
      if (it != winning_record_sources.end())
         if (it->second != &file)
            continue;
      ::load_ref_from_record(record);
   }
}
```

As the pseudocode comments indicate, the `TES4/ONAM` subrecord in the file header is a list of record IDs for temporary refs that are:

* Defined in one of the file's masters
* Non-persistent(?)
* Overridden by the file

The game uses `ONAM` to build a map of ref form IDs to `TESFile`s, so that it knows what file defines the winning record for any given ref that it has to load on demand.
