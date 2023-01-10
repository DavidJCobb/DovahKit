
# ui.progress_bar

This is a subclass of `ui.widget`, and inherits all non-static members.

## Instance methods

<dl>
   <dt>instance:reset()</dt>
   <dd>
      Resets the progress bar to show no progress.
   </dd>
</dl>

## Instance properties

<dl>
   <dt>instance.alignment</dt>
   <dd>
      A string controlling the alignment of the progress bar's embedded text.
   </dd>
   <dt>instance.current_text</dt>
   <dd>
      A read-only string containing the text currently displayed on the progress bar.
   </dd>
   <dt>instance.format</dt>
   <dd>
      <p>A string controlling the format of the progress bar's embedded text. You can use the following format tokens:</p>
      <dl>
         <dt>%m</dt>
         <dd>
            The maximum value.
         </dd>
         <dt>%p</dt>
         <dd>
            The current progress as a percentage.
         </dd>
         <dt>%v</dt>
         <dd>
            The current value.
         </dd>
      </dl>
      <p>For long-running tasks, where individual items are completed quickly but there are a <em>lot</em> of items to complete, it may be more user-friendly to display the current value rather than the percentage. If you have to process 200,000 items and you can process them at a rate of 1000 per second, then what looks faster to the user? A counter that increases by 1000 per second, or a percentage that increases by 0.5% per second?</p>
   </dd>
   <dt>instance.maximum</dt>
   <dd>
      A number indicating the progress bar's maximum value. Non-integer values will be truncated.
   </dd>
   <dt>instance.minimum</dt>
   <dd>
      A number indicating the progress bar's minimum value. Non-integer values will be truncated.
   </dd>
   <dt>instance.show_text</dt>
   <dd>
      A boolean controlling whether the progress bar shows overlaid text to display its value.
   </dd>
   <dt>instance.value</dt>
   <dd>
      A number indicating the progress bar's current value. Non-integer values will be truncated.
   </dd>
</dl>

## Static methods

<dl>
   <dt>ui.progress_bar.is(arg)</dt>
   <dd>
      Tests whether <var>arg</var> is an instance of this class. Returns a boolean.
   </dd>
   <dt>ui.progress_bar.new()</dt>
   <dd>
      <p>Creates and returns a new instance. Passing any arguments will throw an error.</p>
   </dd>
</dl>