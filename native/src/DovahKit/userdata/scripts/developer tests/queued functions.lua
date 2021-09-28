window = ui.window.new()
window.title = "Test"

dummy_button = ui.button.new("Hover for info")
dummy_button.flat = true
dummy_button.tooltip = "This is a tooltip! :)"

slow_button = ui.button.new("Long operation (bare)")
fast_button = ui.button.new("Long operation (queued)")

progress = ui.progress_bar.new()
progress.minimum = 0
progress.value   = 0

function long_operation()
   local THRESHOLD <const> = 300000
   local INCREMENT <const> = 100
   local PROG_MAX  <const> = THRESHOLD / INCREMENT
   --
   progress.maximum = PROG_MAX
   progress.value   = 0
   --
   dovah.log_message("Operation starting...")
   local out = {}
   for i = 1, THRESHOLD, INCREMENT do
      progress.value = math.floor(i / INCREMENT)
      out[i] = true
      if (i / INCREMENT) % 100 then
         for j = 1, 10 do
            collectgarbage("collect")
         end
      end
   end
   progress.value = PROG_MAX
   --
   local max = nil
   local k,v = next(out)
   while k do
      if (not max) or max < k then
         max = k
      end
      k, v = next(out, k)
   end
   --
   dovah.log_message("Operation complete!")
end

window:set_layout("down")
window:add_child(dummy_button)
window:add_child(progress)
window:add_child(slow_button)
window:add_child(fast_button)

do
   local textbox = ui.textbox.new()
   textbox.placeholder = "Can't type during bare op"
   window:add_child(textbox)
end

function set_enable_state(s)
   slow_button.enabled = s
   fast_button.enabled = s
end

slow_button:on("OnActivated", "", function()
   set_enable_state(false)
   long_operation()
   set_enable_state(true)
end)
fast_button:on("OnActivated", "", function()
   set_enable_state(false)
   ui.run_when_unlocked(function()
      long_operation()
      ui.run_when_locked(function()
         set_enable_state(true)
      end)
   end)
end)

window:show()