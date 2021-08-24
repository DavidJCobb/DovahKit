subject = ui.textbox.new("initial")

subject.max_length  = 50
subject.placeholder = "placeholder text"

subject:on("OnChanged", "", function(v)
   dovah.log_message("event OnChanged: %s", tostring(v))
end)
subject:on("OnInputRejected", "", function()
   dovah.log_message("event OnInputRejected")
end)
subject:on("OnKeyPressed", "", function(v)
   dovah.log_message("event OnKeyPressed: %s", tostring(v))
end)

local window = ui.window.new()
window:set_layout("grid")
window:add_child(subject)
window:show()