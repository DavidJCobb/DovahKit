subject = ui.formpicker.new()

subject:on("OnChanged", "", function(form)
   dovah.log_message("event OnChanged: %s", form)
end)

local window = ui.window.new()
window:set_layout("grid")
window:add_child(subject)
window:show()