
local PATH = "textures/landscape/snow01.dds"

local window = ui.window.new()
local widget = ui.canvas.new()
window:set_layout("grid")
window:add_child(widget)

widget.width  = 64
widget.height = 64

local dds = dovah.lookup_game_asset(PATH)
local layer = widget:append_layer()
layer.data = dds
layer.x = 8

window:show()

if not dds then
   dovah.log_message("DDS missing; did you remember to load any game data?")
end

--
-- THINGS PENDING TESTING:
--
--  - Lifetime tests: can a Lua-referenced layer keep its owning canvas alive 
--    even if that canvas is Lua-unreferenced?
--
-- THINGS NOT YET IMPLEMENTED:
--
--  - Text layers
--