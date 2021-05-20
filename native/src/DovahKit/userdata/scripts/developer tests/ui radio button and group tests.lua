local button1 = ui.radio_button.new("Radio Button #1")
local button2 = ui.radio_button.new("Radio Button #2")

local group = ui.radio_group.new()
button1.group = group
button2.group = group

local logger1 = ui.button.new("Log")
local logger2 = ui.button.new("Log")

logger1:on("OnActivated", "", function()
   dovah.log_message("Selected button in R1's group: %s", tostring(button1.group.selected_id))
   dovah.log_message(" - R1's ID: %s", tostring(button1.id))
   local sb = button2.group.selected_button
   if sb then
      dovah.log_message(" - Selected ID via button: %s", tostring(button2.group.selected_button.id))
   else
      dovah.log_message(" - Selected ID via button: %s", tostring(nil))
   end
end)
logger2:on("OnActivated", "", function()
   dovah.log_message("Selected button in R2's group: %s", tostring(button2.group.selected_id))
   dovah.log_message(" - R2's ID: %s", tostring(button2.id))
   local sb = button2.group.selected_button
   if sb then
      dovah.log_message(" - Selected ID via button: %s", tostring(button2.group.selected_button.id))
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
   local window2 = ui.window.new()

   window1:set_layout("ltr")
   window1:add_child(button1)
   window1:add_child(logger1)
   window2:set_layout("ltr")
   window2:add_child(button2)
   window2:add_child(logger2)

   window1:show()
   window2:show()
end

do -- force-value window
   local window = ui.window.new()
   local picker = ui.spinbox.new()
   local button = ui.button.new("Set")
   window.title = "Forcibly select a button"
   window:set_layout("ltr")
   window:add_child(picker)
   window:add_child(button)
   button:on("OnActivated", "", function()
      group.selected_id = picker.value
      dovah.log_message("Selected ID set to " .. picker.value .. ".")
   end)
   picker.decimals = 0
   picker.minimum = -(picker.maximum - 1)
   window:show()
end