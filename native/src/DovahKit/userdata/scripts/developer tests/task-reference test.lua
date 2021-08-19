local window = ui.window.new()
window:show()

do
   local dummy = ui.window.new()
   dummy:hide()
   --
   -- When (dummy) goes out of scope, it will have ceased to be task-referenced 
   -- and it will cease to be Lua-referenced. Whichever happens first will depend 
   -- on variations in execution speed, though the collectgarbage calls below 
   -- should ensure that it *does* become Lua-unreferenced.
   --
end
collectgarbage("collect")
collectgarbage("collect")