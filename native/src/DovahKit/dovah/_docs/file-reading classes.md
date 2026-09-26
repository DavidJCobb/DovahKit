
# File loaders

The process of scanning through a data file to index its forms as "form stubs" is called "loading the file." In DovahKit's alpha, the class hierarchy for file loaders is complete spaghetti. This is because there's no separation between something that *owns*, something that is a *view*, and something that *does the viewing*.

* The `basic_reader` base class provides the view-like functionality, with a `group`, `record`, and `subrecord` inside.

* `basic_reader`, in turn, is subclassed by `file_or_file_part_loader`, which is responsible for sending loaded data to the `file_load_order`.

* `file_or_file_part_loader` is subclassed by:

  * `file_loader`, which actually owns a file.
  
  * `file_part_loader`, which loads a limited section of that file. (More than one of these may exist for a given `file_loader`, e.g. to load a file using multiple threads.)

This is even more tangled because `basic_reader` queries some state that belongs to `file_loader`, so it's a superclass whose functionality is partially dependent on one of its own subclasses. Plus, since `file_or_file_part_loader` handles communication with the `file_load_order`, it also needs to be aware of the `file_loader` (i.e. its own subclass) in order to both get information about the file (e.g. filename) and to forcibly abort the entire load process (across all extant part-loaders) when a file load error is encountered.

Obviously, this sucks. A post-launch refactor would involve a cleaner separation between "a file," which doesn't offer any functionality related to viewing its contents, and "a file view," which has group, record, and subrecord view interfaces.
