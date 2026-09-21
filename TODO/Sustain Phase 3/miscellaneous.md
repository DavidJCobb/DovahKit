
# Miscellaneous

## Undelete-and-disable ref

ObjectReference undelete-and-disable question: why are actors that are "deleted" in this manner flagged as persistent? Someone used `git blame` on xEdit and found that it dates back to the initial GitHub commit, i.e. when xEdit was migrated from an older source control system; I don't believe any specific reason was specified at the time.

I know offhand that there's some jank involving refs with multiple overrides, when some records are persistent and some are not. I'd have to dig into the cell-/ref-loading code again to disentangle it.
