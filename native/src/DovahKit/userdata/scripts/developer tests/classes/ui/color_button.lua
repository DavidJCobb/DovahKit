subject = ui.color_button.new("#FF0000")

subject:on("OnChanged", "", function(v)
   dovah.log_message("event OnChanged: %s", dovah.deep_stringify(v))
end)

local window = ui.window.new()
window:set_layout("grid")
window:add_child(subject)
window:show()