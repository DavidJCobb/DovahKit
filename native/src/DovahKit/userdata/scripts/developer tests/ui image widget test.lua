
local PATH = "textures/landscape/snow01.dds"

local window = ui.window.new()
local widget = ui.image_widget.new()
window:set_layout("grid")
window:add_child(widget)
widget.image = dovah.load_game_asset(PATH)

widget.min_height = 32
widget.min_width  = 32

window:show()