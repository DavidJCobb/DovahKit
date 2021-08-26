subject = ui.groupbox.new("Hello, world!")
subject.checkable = true

subject:on("OnToggled", "", function(checked)
   dovah.log_message("event OnToggled: %s", tostring(checked))
end)

local window = ui.window.new()
window:set_layout("grid")
window:add_child(subject)
window:show()