
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
-- ISSUES AS OF 6/14/2021:
--
--  - The canvas has no maximum size. When placed in a layout, Qt may give it 
--    more than its maximum size to work with, and it will draw in all of the 
--    available space. For example, a canvas that wants to be a 64x64px image 
--    may be given more than 64x64px of space to work with, and if it has any 
--    layers that extend past 64x64px, it will draw the parts that should be 
--    clipped by that 64x64px boundary.
--
-- THINGS PENDING TESTING:
--
--  - Multiple layers
--
--  - Layer groups (no API for this yet)
--
--  - Changing layer positions and visibility within script via buttons
--
--  - Changing layer data (including to nil) within script via buttons
--
--  - Lifetime tests: can a Lua-referenced layer keep its owning canvas alive 
--    even if that canvas is Lua-unreferenced?
--
-- THINGS NOT YET IMPLEMENTED:
--
--  - Layer blend modes (not implemented within C yet)
--
--  - Layer opacity (not implemented within C yet)
--
--  - Text layers
--