
# Floats packed as ints

It'd be nice to have some sort of generic/metaprogrammable solution for working with these.

## All known cases

### Fixed extents

* Weather
  * Transition Threshold, Intro: range [0, 0.999] packed as a `uint8_t`
  * Transition Threshold, Outro: range [0.001, 1] packed as a `uint8_t`
  * Wind Speed (X and Y): range [-0.1F, 0.1F] packed as a `uint8_t`
    * Custom conversion formulae
      * Float to byte: `v * 10 * 127 + 127`
      * Byte to float: `(v - 127) / 127 / 10`

### Variable extents

Some values' extents depend on Game Settings' values.

* Weather: All Cloud Layers: Speed (X and Y)
  * Range: [-`fWeatherCloudSpeedMax`, `fWeatherCloudSpeedMax`]
  * Defaults: [-0.1, 0.1]
  * Skyrim.esm: [-0.0142875, 0.0142875]
* Weather: Trans Delta
  * Range: [`fTransDeltaMin`, `fTransDeltaMax`]
  * Defaults: [0.01, 0.25]
  * Skyrim.esm: [0.01, 0.25]
