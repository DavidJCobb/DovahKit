local child = ui.button.new()
child.name = "Child with unreferenced parent"
child.text = "Test!"
do
   local parent = ui.widget.new()
   parent.name = "Unreferenced parent"
   parent:add_child(child)
end
collectgarbage("collect")
collectgarbage("collect") -- has to be twice

--
-- The parent has gone out of scope, and we have forced its userdata 
-- to be garbage-collected.
--

local window = ui.window.new()
local button = ui.button.new("Check child")
window:set_layout("grid")
window:add_child(button)

window.name = "Visible window"
button.name = "Visible button"

button:on("OnActivated", "", function()
   --
   -- If we have failed to properly manage widget lifetimes, then the 
   -- child widget will have been deleted by the time this runs, with 
   -- the Lua VM left holding a dangling pointer; that should cause 
   -- this to crash.
   --
   -- If, on the other hand, we did our job, then the child widget 
   -- should still exist and this should be perfectly safe.
   --
   dovah.log_message(child.text)
end)

window:show()