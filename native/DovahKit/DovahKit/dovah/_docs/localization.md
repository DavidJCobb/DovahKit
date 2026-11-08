
# Localization

Some pieces of text in game data are *localized strings*. This means that in a non-localized TES file, these strings' content is encoded in a single language and stored inline within subrecords; but in a localized TES file, the strings' content is stored in separate files, and the subrecords hold a four-byte string ID. The benefit of this system is that game content can be translated without ever actually having to modify or override the game's data files.

Not all strings can be localized; only specific subrecords in forms, and only `GMST/DATA` for string-type game settings, can be localized. Whether content is localized is indicated by a flag in the data file header.

## Notes

* The game uses a shared ID space for all strings; however, strings are divided into three categories, and they're stored in and looked up from different files based on the category. You could hypothetically treat each category as having its own ID space, but Bethesda themselves didn't do so, it's not clear how doing so would affect tagification, and you can't guarantee that other community tools will or won't do so.


## Best practices

### Merging identical strings

When DovahKit converts an active file from non-localized to localized, it's capable of "merging" identical strings so that they use a single string ID. You should be careful about doing this (and generally prefer not to do it), as it may cause unexpected difficulties for anyone wishing to translate your content.

Consider for example the case of a "yes" or "no" question. In English, there are just those two possible answers: "yes" and "no." In German, however, these questions can potentially have three answers: there's "ja" and "nein" as you'd expect, but if the question is a negative, then the opposite of "no" i.e. "nein" is actually "doch." For example, the question "You aren't coming along with us, are you?" presents a negative; it would be answered in English with either "No," to affirm the negative (you aren't coming along), or "Yes, I am," to reject the negative and insist on the contrary positive (you are coming along); a bare "yes" would be ambiguous ("yes, you're right, I'm not coming along" or "yes, I am coming along"). In German, you would answer a negative question like this with either "nein" to affirm the negative, or "doch" to reject the negative. Phrases like "Yes, I am" in English can affirm a positive *or* reject a negative, but German may prefer "ja" or "doch" for those two use cases, and blindly merging all "Yes, I am" strings into a single string ID would cause problems for a German translation.

Only merge a string if you can *guarantee* that it *must* be identical across all languages. For example, Skyrim has multiple "Wine" items that have slightly different models but are otherwise totally identical in function and form, so *most likely*, it would *probably* be acceptable to merge these items' "Wine" names into a single string.

Merging identical strings will not make a strings file smaller. It is already possible for two different string IDs to refer to the same piece of string data within a strings file (i.e. by listing the same file offset). This means that if two different string IDs happen to have the same text within a specific language, that text can be written into the strings file just once, with the two strings still being able to differ from one another in the strings files for other languages.
