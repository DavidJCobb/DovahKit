
# color

Any string or table that meets the appropriate requirements to be recognized as a color. The following formats are permitted:

## Table

If the table has keys "r", "g", "b", or "a", or indices 1, 2, 3, or 4, then these will be used as color components. Named fields are preferred over indexed fields. Missing RGB components are treated as 0; a missing alpha will be treated as 255. Color components outside the range [0, 255] will throw an error.

Examples of valid tables:

* `{ r = 255, g = 0, b = 0 }`
* `{ 255, 0, 0 }`
* `{ 255, 0, b = 0 }`

Otherwise, if the table has a `__tostring` metamethod, then it will be converted to a string and that string will be interpreted.

## String

### Hex colors

Hexadecimal color codes must start with a hash sign. They must contain three, four, six, or eight digits.

* The six-digit version is an RGB color code, where each pair of digits represents R, G, and B, respectively.
* The eight-digit version is an RGBA color code.
* The three-digit version is an abbreviated RGB color code. Each digit is extended: `"#F00"` is equivalent to `"#FF0000"`.
* The four-digit version is an abbreviated RGBA color code.

### rgb(...) colors

A string matching the syntax used by CSS `rgb()` or `rgba()` colors. Examples of valid strings:

* `"rgb(255, 0, 0)"`
* `"rgba(255, 0, 0, 255)"`
* `"rgb(100%, 0%, 0%)"`
* `"rgb(100% 0% 0% / 100%)"`

The RGB values may be specified as percentages or as absolute values, but they must use a consistent format with one another. The alpha component, if specified, can be a percentage or an absolute value. You can separate all components with commas, or you can separate the RGB values with spaces and separate the alpha component with a forward slash.

### hsl(...) colors

A string matching the synatx used by CSS `hsl()` or `hsla()` colors. Examples of valid strings:

* `"hsl(20deg, 50%, 50%)"`
* `"hsla(20deg, 50%, 50%, 100%)"`

The hue must be specified as an angle in the color wheel. Allowed units are `deg`, `rad`, `grad`, and `turn`; if no unit is specified, then degrees are assumed. The saturation and lightness must be specified as percentages. The alpha component, if specified, can be a percentage or an absolute value. You can separate all components with commas, or you can separate the RGB values with spaces and separate the alpha component with a forward slash.