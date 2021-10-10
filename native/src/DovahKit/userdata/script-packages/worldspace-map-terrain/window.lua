local FileOutlineWidget = {}
FileOutlineWidget.__index = FileOutlineWidget
do -- FileOutlineWidget contents
   function FileOutlineWidget:new(file_map)
      local instance = setmetatable({}, self)
      --
      instance.widget    = ui.widget.new()
      instance.file_map  = file_map
      instance._controls = {
         root  = instance.widget,
         check = nil,
         color = nil,
      }
      do
         local r = instance.widget
         r.layout_margins = 0
         r:set_layout("grid")
         local c = ui.checkbox.new(file_map.filename)
         c.checked = true
         c:on("OnToggled", "", function(checked)
            instance:on_toggled(checked)
         end)
         instance._controls.check = c
         r:add_child(c, 1, 1)
         --
         -- TODO: color picker
      end
      --
      return instance
   end
   function FileOutlineWidget:on_toggled(checked)
      self.file_map:set_visible(checked)
   end
end

--

HeightmapWindow = {
   controls = {
      window   = ui.window.new(),
      scroll   = ui.scrollbox.new(),
      canvas   = ui.canvas.new(),
      progress = ui.progress_bar.new(),
      --
      config   = ui.widget.new(),
      bottom   = ui.widget.new(),
      --
      options = {
         world            = false,
         layer_visibility = {}, -- list of checkboxes
         water_color      = false,
         execute          = false,
         --
         outline_toggle_holder = false,
      },
      outline_toggles = {},
   },
   state = {
      layers = {},
      layer_visibility = {},
   },
}
do -- HeightmapWindow contents
   do -- Initial config for HeightmapWindow.
      for i = 1, #LAYER_SPEC do
         local spec = LAYER_SPEC[i]
         local name = spec.name
         HeightmapWindow.state.layer_visibility[name] = true
      end
   end
   do -- Create widgets and layout for HeightmapWindow.
      local window = HeightmapWindow.controls.window
      window.title = "Heightmap"
      window:set_layout("grid")
      window.width  = 640
      window.height = 400
      --
      local scroll = HeightmapWindow.controls.scroll
      local config = HeightmapWindow.controls.config
      window:add_child(scroll, 1, 1)
      window:add_child(config, 1, 2)
      window:set_layout_stretch_at("col", 1, 1)
      window:set_layout_stretch_at("col", 2, 0)
      do
         local sb = scroll.body
         sb:set_layout("grid")
         sb:add_child(HeightmapWindow.controls.canvas)
      end
      --
      do
         local bottom = HeightmapWindow.controls.bottom
         window:add_child(bottom, 2, 1, 1, 2)
         bottom:set_layout("down")
         bottom.layout_margins = 0
         --
         local progress = HeightmapWindow.controls.progress
         bottom:add_child(progress)
         progress.alignment = "center center"
         --
         do
            local tip = ui.text.new("TIP: It's normal for rivers and other water formations to be missing chunks. In order to allow for waterfalls at oblique angles relative to the compass, Bethesda will use placed water objects instead of cell and worldspace water.")
            tip.word_wrap = true
            bottom:add_child(tip)
         end
      end
      --
      do -- Create options
         config:set_layout("down")
         config.layout_margins = 0
         --
         local oc = HeightmapWindow.controls.options
         local lv = oc.layer_visibility
         do
            local picker = ui.formpicker.new()
            picker.form_types   = form_types.worldspace
            picker.default_form = dovah.get_form_by_id(0x3C)
            picker.allow_none   = false
            config:add_child(picker)
            --
            oc.world = picker
         end
         for i = 1, #LAYER_SPEC do
            local spec = LAYER_SPEC[i]
            local name = spec.name
            local text = spec.check_text
            --
            lv[name] = ui.checkbox.new(text)
            lv[name].checked = true
            lv[name]:on("OnToggled", "", function(checked)
               HeightmapWindow:set_layer_visibility(name, checked)
            end)
            config:add_child(lv[name])
         end
         do
            local wrap = ui.widget.new()
            wrap:set_layout("grid")
            wrap.layout_margins = 0
            wrap:add_child(ui.text.new("Water color:"), 1, 1)
            --
            local picker = ui.color_button.new()
            picker.color = "#43618B"
            picker:on("OnChanged", "", function(color)
               local layer = HeightmapWindow.state.layers["water"]
               if layer then
                  local data = layer.data
                  if data and dovah.type(data) == "raster" then
                     data:fill_rgb(color)
                  end
               end
            end)
            wrap:add_child(picker, 1, 2)
            config:add_child(wrap)
            --
            oc.water_color = picker
         end
         do
            local button = ui.button.new("Render")
            config:add_child(button)
            --
            oc.execute = button
         end
         --
         config:add_child(ui.line.new("h"))
         --
         do
            local widget = ui.text.new("Cell outlines:")
            widget.font.bold = true
            config:add_child(widget)
         end
         do
            local widget = ui.widget.new()
            widget:set_layout("down")
            HeightmapWindow.controls.options.outline_toggle_holder = widget
            --
            config:add_child(widget)
         end
         --
         config:add_spacer("v")
      end
   end
   function HeightmapWindow:clear_canvas()
      local canvas = self.controls.canvas
      canvas.width  = 1
      canvas.height = 1
      local layers = {}
      do
         local source = canvas.layers
         local count  = #source
         for i = 1, count do
            layers[i] = source[i]
         end
      end
      for i = 1, #layers do
         canvas:remove_layer(layers[i])
      end
      self.state.layers = {}
   end
   function HeightmapWindow:clear_cell_outline_toggles()
      local list   = self.controls.outline_toggles
      local parent = self.controls.options.outline_toggle_holder
      for i = 1, #list do
         parent:remove_child(list[i].widget)
         list[i] = nil
      end
   end
   function HeightmapWindow:import_cell_outline_data(all_files_map)
      local list   = self.controls.outline_toggles
      local parent = self.controls.options.outline_toggle_holder
      all_files_map:for_each_map(function(map)
         if map:has_any_islands() then
            local cls = FileOutlineWidget:new(map)
            list[#list + 1] = cls
            parent:add_child(cls.widget)
         end
      end)
   end
   function HeightmapWindow:set_is_locked(state)
      self.controls.config.enabled = not state
   end
   function HeightmapWindow:set_layer_visibility(name_to_alter, state)
      local lowest = nil
      for i = 1, #LAYER_SPEC do
         local spec  = LAYER_SPEC[i]
         local name  = spec.name
         local layer = HeightmapWindow.state.layers[name]
         local show  = HeightmapWindow.state.layer_visibility[name]
         if name == name_to_alter then
            if layer then -- user can configure settings before rendering i.e. before layers exist
               layer.visible = state
            end
            HeightmapWindow.state.layer_visibility[name] = state
            show = state
         end
         if layer then -- user can configure settings before rendering i.e. before layers exist
            if show and not lowest then
               lowest = layer
               layer.blend_mode = "normal"
               layer.opacity    = 1
            else
               if spec.blend_mode then
                  layer.blend_mode = spec.blend_mode
               end
               if spec.opacity then
                  layer.opacity = spec.opacity
               end
            end
         end
      end
   end
   function HeightmapWindow:show()
      self.controls.window:show()
   end
end

HeightmapWindow.controls.options.execute:on("OnActivated", "render", function()
   HeightmapWindow:set_is_locked(true)
   HeightmapWindow:clear_canvas()
   HeightmapWindow:clear_cell_outline_toggles()
   render_worldspace_height(
      HeightmapWindow.controls.options.world.form,
      HeightmapWindow.controls.options.water_color.color
   )
   HeightmapWindow:set_is_locked(false)
end)