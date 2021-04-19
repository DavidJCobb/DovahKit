local window = ui.window.new()
local checkb = ui.checkbox.new("Checkbox")
local groupb = ui.groupbox.new("Groupbox")
local set_in = ui.button.new("Set Indeterminate")

window:set_layout("ltr")
groupb:set_layout("down")

groupb.checkable = true

window:add_child(groupb)
groupb:add_child(checkb)
groupb:add_child(set_in)

checkb:on("OnChanged", "", function(state)
   dovah.log_message("Checkbox state: %s", state)
end)
checkb:on("OnToggled", "", function(checked)
   dovah.log_message("Checkbox checked: %s", tostring(checked))
end)
groupb:on("OnToggled", "", function(checked)
   dovah.log_message("Groupbox checked: %s", tostring(checked))
end)
set_in:on("OnActivated", "", function()
   checkb.state = "indeterminate"
end)

window:show()