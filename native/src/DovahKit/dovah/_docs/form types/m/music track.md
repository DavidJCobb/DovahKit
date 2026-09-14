
# Music track

If a music *type* is a collection of related "fragments" of music, then a music *track* is one of those fragments. A music track can be a single piece of music, a piece of silence, or a "palette" of other music tracks.

## Track types

### Single

A "single"-type music track has a main audio file and optionally a finale audio file. The track can be defined to loop a specific part of its main audio file, and it can be given a list of "cue points" at which it would be appropriate to switch from the main audio to the finale audio.

### Silent

A "silent"-type music track defines a period of silence with a specific fixed length. These tracks aren't terribly useful on their own, but they can be included within music types and "palette"-type music tracks to space other tracks apart during playback.

### Palette

A "palette"-type music track defines up to three "layers" of music, with each layer being a list of other music tracks. The layers play simultaneously.

## Implementation

This form type uses [class names identified by hashes](../../form%20type%20common%20behaviors/class%20names%20identified%20by%20hashes.md) to store most of its data. Three possible classes exist for its typed data: `BGSMusicSingleTrack`, `BGSMusicSilenceTrack`, and `BGSMusicPaletteTrack`.