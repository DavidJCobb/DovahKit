
# Benchmark

A class for precise time measurements, useful for tracking the performance of a script's code. Each instance acts as a kind of stopwatch, and measurement begins the moment the instance is created.

## Static methods

<dl>
   <dt>benchmark.new()</dt>
   <dd>
      Creates and returns a new benchmark instance.
   </dd>
   <dt>benchmark.is(arg)</dt>
   <dd>
      Tests whether <var>arg</var> is an instance of this class. Returns a boolean.
   </dd>
</dl>

## Instance methods

<dl>
   <dt>instance:restart()</dt>
   <dd>
      If the timer has been stopped, restarts it. Does nothing otherwise.
   </dd>
   <dt>instance:stop()</dt>
   <dd>
      Stops the timer.
   </dd>
   <dt>instance:time_to_string()</dt>
   <dd>
      Returns a human-readable string displaying the time in both milliseconds and microseconds. If the timer hasn't been stopped, returns <code>"still running"</code>.
   </dd>
</dl>

## Instance properties

<dl>
   <dt>instance.microseconds</dt>
   <dd>
      If the timer is stopped, returns the elapsed time in microseconds, as an integer. If the timer is running, returns nil. This value is read-only.
   </dd>
   <dt>instance.milliseconds</dt>
   <dd>
      If the timer is stopped, returns the elapsed time in milliseconds, as an integer. If the timer is running, returns nil. This value is read-only.
   </dd>
</dl>