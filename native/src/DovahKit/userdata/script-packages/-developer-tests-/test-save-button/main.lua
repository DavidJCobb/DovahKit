window = ui.window.new()
saver1 = ui.file_save_button.new()
saver2 = ui.file_save_button.new()

window:set_layout("grid")
window:add_child(saver1)
window:add_child(saver2)
window:show()

data = binary_view.new()
data:append_uint32(0x11223344, "big")
data:append_uint8 (0x55)
data:append_uint8 (0x66)
data:append_uint8 (0x77)
data:append_uint8 (0x88)

saver1.data     = data
saver1.filename = "test.exe"
saver1.label    = "test file"

saver2.data     = dovah.package.load_file("YOUR BURDENS.png")
saver2.filename = "file.dds"
saver2.label    = "test image"