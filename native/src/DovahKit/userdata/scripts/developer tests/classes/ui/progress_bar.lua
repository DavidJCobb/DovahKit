subject = ui.progress_bar.new()

local window = ui.window.new()
window:set_layout("grid")
window:add_child(subject)
window:show()