
# alignment

Any string that meets the requirements described below.

## Format

The string can contain a horizontal alignment value, optionally followed by a space and a vertical alignment value. Values are case-insensitive. If a vertical alignment is not provided, then it defaults to `"unchanged"`.

### Horizontal alignment values

<dl>
   <dt>unchanged</dt>
   <dd>
      If this value is used, then the horizontal alignment is not changed.
   </dd>
   <dt>left</dt>
   <dd>
      Left-aligned text.
   </dd>
   <dt>right</dt>
   <dd>
      Right-aligned text.
   </dd>
   <dt>center</dt>
   <dd>
      Centered text.
   </dd>
   <dt>justify</dt>
   <dd>
      Justified text: extra spacing is inserted between words so that the ends of lines match up with one another.
   </dd>
   <dt>start</dt>
   <dd>
      Text aligned to the start of the inline axis. For a left-to-right language, this is left-aligned; for a right-to-left language, it's right-aligned.
   </dd>
   <dt>end</dt>
   <dd>
      Text aligned to end start of the inline axis. For a left-to-right language, this is right-aligned; for a right-to-left language, it's left-aligned.
   </dd>
</dl>

### Vertical alignment values

<dl>
   <dt>unchanged</dt>
   <dd>
      If this value is used, then the vertical alignment is not changed.
   </dd>
   <dt>top</dt>
   <dd>
      Top-aligned text.
   </dd>
   <dt>bottom</dt>
   <dd>
      Bottom-aligned text.
   </dd>
   <dt>baseline</dt>
   <dd>
      Text aligned on the baseline.
   </dd>
   <dt>center</dt>
   <dd>
      Centered text.
   </dd>
</dl>