local window  = ui.window.new()
local input   = ui.textbox.new()
local show_kp = ui.text.new()
local show_c  = ui.text.new()
window:set_layout("grid")
window:add_child(input, 1, 1, 1, 2)
do
   window:add_child(ui.text.new("AfterChanged:"), 2, 1)
   window:add_child(ui.text.new("OnChanged:"),   3, 1)
   window:add_child(show_c,  2, 2)
   window:add_child(show_kp, 3, 2)
   --
   window:set_layout_stretch_at("col", 1, 1)
   window:set_layout_stretch_at("col", 2, 0)
end

input.placeholder = "Test!"
input:on("AfterChanged", "", function(text)
   show_c.text = text
end)
input:on("OnChanged", "", function(text)
   show_kp.text = text
end)
show_c.text, show_kp.text = input.placeholder, input.placeholder

-- Test removing event listeners.
input:on("OnKeyPressed", "test", function(text)
   show_c.text = "ABC"
end)
input:remove_event_listener("OnKeyPressed", "test")

-- Test manually (un)locking the UI.
local button = ui.button.new("Run Long Operation")
button:on("OnActivated", "", function()
   button.enabled = false
   ui.run_when_unlocked(function()
      dovah.log_message("UI unlocked; starting op...")
      local count = 0
      dovah.for_each_form_of_type(form_types.reference, function(form)
         if (form.flags & 0x400) ~= 0 then
            count = count + 1
         else
            local file = form:get_last_source_file()
            if file then
               if not file.is_master then
                  count = count + 1
               end
            else
            end
         end
      end)
      ui.run_when_locked(function()
         dovah.log_message("op done; UI locked.")
         button.enabled = true
         dovah.log_message("button enabled (verify: %s)", tostring(button.enabled))
      end)
   end)
end)
window:add_child(button, 4, 1, 1, 2)

window:show()