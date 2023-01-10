
# font

The font used by certain UI widgets, or by text data in canvas layers. You cannot create instances of this userdata directly, but you also don't need to; if any property has this as a value and is not read-only, then you're allowed to overwrite it with a table that "looks like" one of these.

## Instance properties

<dl>
   <dt>bold</dt>
   <dd>
      A boolean indicating whether the font is bold, or nil to indicate that this property is unchanged. If the font weight is 57% or greater, then the font is considered bold.
   </dd>
   <dt>capitalization</dt>
   <dd>
      <p>A string indicating changes to the letter case of displayed text, or nil to indicate that this property is unchanged. The following case-insensitive string values are allowed:</p>
      <ul>
         <li>normal</li>
         <li>capitalize</li>
         <li>lowercase</li>
         <li>small caps</li>
         <li>uppercase</li>
      </ul>
   </dd>
   <dt>family</dt>
   <dd>
      <p>A string indicating the typeface(s) or generic typeface style of displayed text, or nil to indicate that this property is unchanged. You may write any number of values, separated by commas, and optionally enclosed in quotation marks (single or double). The following generic style names are recognized:</p>
      <ul>
         <li>cursive</li>
         <li>fantasy</li>
         <li>monospace</li>
         <li>sans-serif</li>
         <li>serif</li>
      </ul>
      <p>Additionally, the name of any specific typeface may be used, though it will only work if the user has it installed on their system. The first match in the list is used.</p>
   </dd>
   <dt>italics</dt>
   <dd>
      A boolean indicating whether the font is italic, or nil to indicate that this property is unchanged.
   </dd>
   <dt>letter_spacing</dt>
   <dd>
      A number indicating the number of extra pixels' worth of spacing to insert between characters, or nil to indicate that this property is unchanged. Negative numbers pull letters closer together.
   </dd>
   <dt>overline</dt>
   <dd>
      A boolean indicating whether the text is overlined, or nil to indicate that this property is unchanged.
   </dd>
   <dt>size</dt>
   <dd>
      A string indicating the font size in either pixels or points, depending on the suffix used ("px" or "pt"); or nil to indicate that this property is unchanged. The number must be an integer. Whitespace is permitted around the value, and between the number and the unit. Negative font sizes are not permitted. As of this writing, the maximum allowed font size is 1000px or 1000pt, whichever is larger.
   </dd>
   <dt>strikethrough</dt>
   <dd>
      A boolean indicating whether the text has a line crossing it out, or nil to indicate that this property is unchanged.
   </dd>
   <dt>underline</dt>
   <dd>
      A boolean indicating whether the text is underlined, or nil to indicate that this property is unchanged.
   </dd>
   <dt>weight</dt>
   <dd>
      An integer between 1 and 100, inclusive, indicating the boldness of the font; or nil to indicate that this property is unchanged.
   </dd>
   <dt>width</dt>
   <dd>
      An integer between 1 and 4000, inclusive, indicating to what extent the font is stretched, where 100 is the default; or nil to indicate that this property is unchanged.
   </dd>
   <dt>word_spacing</dt>
   <dd>
      A number indicating the number of extra pixels' worth of spacing to insert between words, or nil to indicate that this property is unchanged. Negative numbers pull words closer together. This does not apply to space-less writing systems.
   </dd>
</dl>