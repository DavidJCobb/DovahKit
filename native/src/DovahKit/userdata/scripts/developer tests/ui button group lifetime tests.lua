button1 = ui.radio_button.new("Radio Button #1")
button2 = ui.radio_button.new("Radio Button #2")

group = ui.radio_group.new()
button1.group = group
button2.group = group

local logger1 = ui.button.new("Log")

logger1:on("OnActivated", "", function()
   dovah.log_message("Selected button in R1's group: %s", tostring(button1.group.selected_id))
   dovah.log_message(" - R1's ID: %s", tostring(button1.id))
   local sb = button1.group.selected_button
   if sb then
      dovah.log_message(" - Selected ID via button: %s", tostring(button1.group.selected_button.id))
   else
      dovah.log_message(" - Selected ID via button: %s", tostring(nil))
   end
end)

group:on("OnSelectionChanged", "", function(button)
   dovah.log_message("Group:OnSelectionChanged: %s", button)
   if button then
      dovah.log_message(" - %s", button.text)
   end
end)
button1:on("OnChanged", "", function(state)
   dovah.log_message("Button 1 state: %s", state)
end)
button1:on("OnToggled", "", function(state)
   dovah.log_message("Button 1 toggled: %s", state)
end)

do
   local window1 = ui.window.new()

   window1:set_layout("ltr")
   window1:add_child(button1)
   window1:add_child(logger1)

   window1:show()
end

button2 = nil
group   = nil
collectgarbage("collect")
collectgarbage("collect")

-- no dialogs or logging for this test; use breakpoints in C to see 
-- if button groups are being GC'd properly (or rather, for this test, 
-- to see if they're being not-GC'd properly).