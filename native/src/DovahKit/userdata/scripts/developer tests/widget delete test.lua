local window = ui.window.new()
do
   local target = ui.button.new("TARGET")
   local button = ui.button.new("Delete TARGET Button")

   window:set_layout("grid")
   window:add_child(target, 1, 1)
   window:add_child(button, 1, 2)

   button:on("OnActivated", "listener", function()
      window:remove_child(target)
      target = nil
      button:remove_event_listener("OnActivated", "listener")
      collectgarbage("collect") -- force GC ASAP
      collectgarbage("collect") -- force GC ASAP
   end)
end
window:show()