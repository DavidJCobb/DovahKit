window = ui.window.new()
widget = ui.image_widget.new()
window:set_layout("grid")
window:add_child(widget)

widget.image = dovah.package.load_file("YOUR BURDENS.png")

window:show()