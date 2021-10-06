local BASE_FORMS = {
   form_types.sound,
   form_types.activator,
   form_types.armor,
   form_types.book,
   form_types.container,
   form_types.door,
   form_types.ingredient,
   form_types.light,
   form_types.misc_item,
   form_types.apparatus,
   form_types.static,
   form_types.movable_static,
   form_types.tree,
   form_types.flora,
   form_types.furniture,
   form_types.weapon,
   form_types.ammo,
   form_types.actor_base,
   form_types.leveled_character,
   form_types.key,
   form_types.potion,
   form_types.idle_marker,
   form_types.note,
   form_types.soul_gem,
   form_types.leveled_item,
}

-- List for setting which forms to render
FormpickerList = false
do
   FormpickerList = {
      widgets = {
         outer = false,
         inner = false,
      },
      pickers = {},
      button  = {},
   }
   
   do
      local o = ui.widget.new()
      local i = ui.widget.new()
      FormpickerList.widgets.outer = o
      FormpickerList.widgets.inner = i
      o:set_layout("down")
      i:set_layout("down")
      o.layout_margins = 0
      i.layout_margins = 0
      o.min_width = 275
      do
         local widget = ui.text.new("Base forms to find:")
         widget.font.bold = true
         o:add_child(widget)
      end
      o:add_child(i)
      --
      local b = ui.button.new("+")
      o:add_child(b)
      o:add_spacer("v")
      --
      FormpickerList.button = b
   end
   
   function FormpickerList:add_picker(row_object)
      self.widgets.inner:add_child(row_object.widget)
      --
      local list = self.pickers
      list[#list + 1] = row_object
   end
   function FormpickerList:append_to_widget(widget, ...)
      widget:add_child(self.widgets.outer, ...)
   end
   function FormpickerList:get_forms()
      local set  = {}
      local list = {}
      for i = 1, #self.pickers do
         local form = self.pickers[i]:get_form()
         if form and not set[form] then
            set[form]       = true
            list[#list + 1] = form
         end
      end
      return list
   end
   function FormpickerList:set_enable_state(state)
      self.widgets.outer.enabled = state
   end
   function FormpickerList:remove(row_object)
      local list = {}
      local j    = 0
      for i = 1, #self.pickers do
         local item = self.pickers[i]
         if item ~= row_object then
            j = j + 1
            list[j] = item
         end
      end
      self.pickers = list
   end
   
   local _Row = {}
   _Row.__index = _Row
   do
      function _Row:new()
         local instance = setmetatable({}, self)
         local w = ui.widget.new()
         local f = ui.formpicker.new()
         local x = ui.button.new("X")
         --
         w:set_layout("ltr")
         w.layout_margins = 0
         w:add_child(f)
         w:add_child(x)
         --
         f.allow_none = true
         f.form_types = BASE_FORMS
         f:on("OnChanged", "", function(form)
         end)
         --
         x.max_width = 32
         x:on("OnActivated", "", function()
            FormpickerList:remove(instance)
            local p = w.parent
            if p then
               p:remove_child(w)
            end
         end)
         --
         instance.widget = w
         instance.picker = f
         instance.button = x
         --
         FormpickerList:add_picker(instance)
         return instance
      end
      function _Row:get_form()
         return self.picker.form
      end
   end
   
   FormpickerList.button:on("OnActivated", "add row", function()
      _Row:new()
   end)
end

-- Widget for toggling visibility and setting color on rendered forms
local FormMapRow = {}
FormMapRow.__index = FormMapRow
do
   function FormMapRow:new(refr_group)
      local instance = setmetatable({}, self)
      --
      instance.widget     = ui.widget.new()
      instance.refr_group = refr_group
      instance._controls  = {
         root  = instance.widget,
         check = nil,
         color = nil,
      }
      do
         local root = instance.widget
         root:set_layout("ltr")
         root.layout_margins = 0
         --
         local text = ""
         do
            local form = refr_group.form
            local id   = form:form_id_to_string()
            local ed   = form.editor_id or "<unnamed>"
            text = string.format("%s (%s)", ed, id)
         end
         local c = ui.checkbox.new(text)
         c.checked = true
         c:on("OnToggled", "", function(state) instance:on_toggled(state) end)
         instance._controls.check = c
         root:add_child(c)
         --
         c = ui.color_button.new()
         c.color = refr_group.color
         c:on("OnChanged", "", function(color) instance:on_color_changed(color) end)
         instance._controls.color = c
         root:add_child(c)
         --
         root:set_layout_stretch_at(1, 1)
         root:set_layout_stretch_at(2, 0)
      end
      --
      return instance
   end
   function FormMapRow:on_toggled(checked)
      self.refr_group:set_visible(checked)
   end
   function FormMapRow:on_color_changed(color)
      self.refr_group:set_color(color)
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
         world   = false,
         cache   = false,
         execute = false,
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
      window.title = "Map renderer"
      window:set_layout("grid")
      window.width  = 640
      window.height = 400
      --
      local scroll = HeightmapWindow.controls.scroll
      local config = HeightmapWindow.controls.config
      window:add_child(scroll, 1, 1)
      window:add_child(config, 1, 2)
      window:set_layout_stretch_at("col", 1, 3)
      window:set_layout_stretch_at("col", 2, 1)
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
         do
            local picker = ui.formpicker.new()
            picker.form_types   = form_types.worldspace
            picker.default_form = dovah.get_form_by_id(0x3C)
            picker.allow_none   = false
            config:add_child(picker)
            --
            oc.world = picker
         end
         do -- cache references of type
            local checkbox = ui.checkbox.new("Cache forms during search")
            config:add_child(checkbox)
            checkbox:on("OnToggled", "", function(checked)
               if not checked then
                  local world = oc.world.form
                  if not world then
                     return
                  end
                  WorldMapper:forget_world(world)
               end
            end)
            checkbox.whats_this = "The first map drawn for a worldspace will create a cache, indexing all placed objects by their base form type. Subsequent maps will draw much more quickly, but the cache may take a lot of memory. Once a placed object's data is loaded (to get its position, if it's relevant to a map), the cache will prevent it from unloading until this window is closed."
            --
            oc.cache = checkbox
         end
         config:add_child(ui.line.new("h"))
         FormpickerList:append_to_widget(config)
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
            local widget = ui.text.new("Base form layers:")
            widget.font.bold = true
            config:add_child(widget)
         end
         do
            local widget = ui.widget.new()
            widget:set_layout("down")
            widget.layout_margins = 0
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
   function HeightmapWindow:clear_generated_map_toggles()
      local list   = self.controls.outline_toggles
      local parent = self.controls.options.outline_toggle_holder
      for i = 1, #list do
         parent:remove_child(list[i].widget)
         list[i] = nil
      end
   end
   function HeightmapWindow:generate_map_toggles(all_files_map)
      local list   = self.controls.outline_toggles
      local parent = self.controls.options.outline_toggle_holder
      all_files_map:for_each_map(function(map)
         local cls = FormMapRow:new(map)
         list[#list + 1] = cls
         parent:add_child(cls.widget)
         if not map:has_any_islands() then
            cls.widget.enabled    = false
            cls.widget.whats_this = "No objects of this type were found in this worldspace."
         end
      end)
   end
   function HeightmapWindow:set_is_locked(state)
      self.controls.config.enabled = not state
   end
   function HeightmapWindow:show()
      self.controls.window:show()
   end
end

HeightmapWindow.controls.options.execute:on("OnActivated", "render", function()
   HeightmapWindow:set_is_locked(true)
   HeightmapWindow:clear_canvas()
   HeightmapWindow:clear_generated_map_toggles()
   local maps = WorldMapper:find_all_in(
      HeightmapWindow.controls.options.world.form,
      FormpickerList:get_forms(),
      HeightmapWindow.controls.canvas,
      HeightmapWindow.controls.progress,
      HeightmapWindow.controls.options.cache.checked
   )
   HeightmapWindow:generate_map_toggles(maps)
   --[[
   render_refr_locations(
      HeightmapWindow.controls.options.world.form,
      FormpickerList:get_forms()
   )
   ]]--
   HeightmapWindow:set_is_locked(false)
end)