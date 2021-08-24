subject = ui.spinbox.new()

subject:on("OnChanged", "", function(v)
   dovah.log_message("event OnChanged: %s", tostring(v))
end)

local window = ui.window.new()
window:set_layout("grid")
window:add_child(subject)
window:show()