
Window = {
   controls = {
      window   = ui.window.new(),
      scroll   = ui.scrollbox.new(),
      canvas   = ui.canvas.new(),
      progress = ui.progress_bar.new(),
      
      config = ui.widget.new(),
      
      buttons = {
         execute = nil,
      },
   }
}

function Window:show()
   self.controls.window:show()
end

do -- set up UI
   local window = Window.controls.window
   window.title = "NavMeshInfoMap"
   window:set_layout("grid")
   window.width  = 640
   window.height = 400
   
   local scroll = Window.controls.scroll
   local config = Window.controls.config
   window:add_child(scroll, 1, 1)
   window:add_child(config, 1, 2)
   window:set_layout_stretch_at("col", 1, 1)
   window:set_layout_stretch_at("col", 2, 0)
   do
      local sb = scroll.body
      sb:set_layout("grid")
      sb:add_child(Window.controls.canvas)
   end
   window:add_child(Window.controls.progress, 2, 1, 1, 2)
   
   do -- options
      config:set_layout("down")
      config.layout_margins = 0
      
      do
         local button = ui.button.new("Render")
         config:add_child(button)
         Window.controls.buttons.execute = button
      end
      
      config:add_spacer("v")
   end
end