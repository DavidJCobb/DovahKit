
TRANSPARENT     = "#00000000"
WATER_DEPTH     = 4096
WATER_MIN_ALPHA = 0.5

CELL_OUTLINE_FILL_OPACITY = 30 -- [0, 255]

-- Comprehensive but slow; haven't yet found cases where it's actually needed
local TEST_FOR_PERSISTENT_REFS = false

local CELL_SIDE_SIZE = 4096
local DEFAULT_LAND = { -- executable-level defaults: a LandTexture created at run-time with no form ID
   diffuse  = nil,
   normal   = nil,
   material = nil, -- TODO: set this to Default Object "DLMT"
}
do
   local s = dovah.lookup_game_ini_setting("Skyrim.ini", "Landscape", "sDefaultLandDiffuseTexture")
   if s then
      DEFAULT_LAND.diffuse = "Landscape\\" .. s.current_value
   else
      error("Unable to get the default land diffuse texture. Failed to access game INI settings.")
   end
   s = dovah.lookup_game_ini_setting("Skyrim.ini", "Landscape", "sDefaultLandNormalTexture")
   if s then
      DEFAULT_LAND.normal = "Landscape\\" .. s.current_value
   else
      error("Unable to get the default land diffuse texture. Failed to access game INI settings.")
   end
end

-- List of internal names for layers. The render process refers to layers 
-- directly and will expect them to exist, but UI setup processes and 
-- similar rely on this struct to create, arrange, and configure layers, 
-- as well as for associated UI widgets e.g. toggle checkboxes.
--
-- Additionally, we can configure a limited range of layer properties such 
-- as the opacity and blend mode, and we can control layer order as well 
-- (first = lowest). If we want to configure additional properties, we 
-- must update the code that creates layers and the code that toggles a 
-- layer's visibility.
--
-- The lowest layer will be forced to "normal" blend mode, fully opaque.
local LAYER_SPEC = {
   {
      name       = "height",
      blend_mode = "multiply",
      opacity    = 0.65,
      check_text = "Show heightmap",
   },
   {
      name       = "paint",
      blend_mode = "multiply",
      opacity    = 0.65,
      check_text = "Show landscape paint",
   },
   {
      name       = "color",
      blend_mode = "multiply",
      check_text = "Show vertex colors",
   },
   {
      name       = "water",
      check_text = "Show water",
   },
}

---

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
         world = false,
         layer_visibility = {}, -- list of checkboxes
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
      window.title = "Heightmap"
      window:set_layout("grid")
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
         local l = layers[i]
         if l.delete then
            l:delete() -- accommodation for old engine; TODO: remove this
         else
            canvas:remove_layer(l)
         end
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

---

TextureManager = {
   map = {},
}
do -- TextureManager contents
   function _try_get_smallest_mip(dds)
      local image = dds.images[1]
      if not image then
         return
      end
      local mips = image.mipmaps
      local last = #mips
      local data = mips[last]
      if data then
         return data:copy_to_raster()
      end
   end
   function _resize_dds(dds)
      local ras = _try_get_smallest_mip(dds)
      if not ras then
         ras = dds:copy_to_raster()
      end
      ras:resize(1, 1)
      return ras
   end
   function _adjust_color(color)
      return
   end
   
   function TextureManager:get_color(land_texture)
      local ts = land_texture.texture_set
      if not ts then
         return TRANSPARENT
      end
      local path = ts.textures.diffuse
      if not path or path == "" then
         return TRANSPARENT
      end
      if self.map[path] then
         return self.map[path]
      end
      --
      local dds <close> = dovah.lookup_game_asset("textures/" .. path)
      if dovah.type(dds) ~= "dds_resource" then
         self.map[path] = TRANSPARENT
         return TRANSPARENT
      end
      local raster <close> = _resize_dds(dds)
      --
      local color = raster:get_pixel(1, 1)
      _adjust_color(color)
      color.a  = 255
      color[4] = 255 -- alpha in land textures means something else (parallax?)
      self.map[path] = color
      --dovah.log_message("Color for %s: (%s, %s, %s, %s)", path, color.r, color.g, color.b, color.a)
      return color
   end
   function TextureManager:get_default_color()
      if self.default_color then
         return self.default_color
      end
      local path = DEFAULT_LAND.diffuse
      --
      local dds <close> = dovah.lookup_game_asset("textures/" .. path)
      if dovah.type(dds) ~= "dds_resource" then
         self.default_color = TRANSPARENT
         return TRANSPARENT
      end
      local raster <close> = dds:copy_to_raster()
      raster:resize(1, 1)
      --
      local color = raster:get_pixel(1, 1)
      _adjust_color(color)
      color.a  = 255
      color[4] = 255 -- alpha in land textures means something else (parallax?)
      self.default_color = color
      return color
   end
end

---

--
-- We want to be able to render outlines around all cells that have 
-- been added or modified by a given file in the load order. The 
-- naive approach would be to use a single layer for each file, but 
-- that's wasteful: if a file were to edit two cells on opposite 
-- corners of a worldspace, then we'd have a tremendous region of 
-- transparent pixels between them.
--
-- Instead, we'll take advantage of layer groups. Each file will 
-- get its own layer group, and within that layer group, we'll have 
-- one layer for each region, each "island," of contiguous cells. 
-- But that means we have to identify and track the islands...
--

local CellOutlineIsland = {}
CellOutlineIsland.__index = CellOutlineIsland
do
   function CellOutlineIsland:new(owner)
      if not owner then
         error("CellOutlineIsland instances must be owned by a CellOutlineMap")
      end
      local instance = setmetatable({}, self)
      instance.cells  = {}
      instance.owner  = owner
      instance.bounds = { x = { false, false }, y = { false, false } }
      instance.layers = {
         fill = false,
         line = false,
      }
      instance.images = {
         fill = false,
         line = false,
      }
      return instance
   end
   function CellOutlineIsland:accept_cell(x, y)
      local col = self.cells[x]
      if not col then
         col = {}
         self.cells[x] = col
      end
      col[y] = true
      do
         local span = self.bounds.x
         if not span[1] or x < span[1] then
            span[1] = x
         end
         if not span[2] or x > span[2] then
            span[2] = x
         end
      end
      do
         local span = self.bounds.y
         if not span[1] or y < span[1] then
            span[1] = y
         end
         if not span[2] or y > span[2] then
            span[2] = y
         end
      end
   end
   function CellOutlineIsland:create_canvas_data()
      local x_min = self.bounds.x[1] - 1
      local x_max = self.bounds.x[2] + 1
      local y_min = self.bounds.y[1] - 1
      local y_max = self.bounds.y[2] + 1
      --
      self.images.line = raster.new({
         width  = (x_max - x_min + 1) * 32,
         height = (y_max - y_min + 1) * 32,
      })
      self.images.fill = raster.new({
         width  = (x_max - x_min + 1) * 32,
         height = (y_max - y_min + 1) * 32,
      })
      local layer_line = self.owner.layer_groups.line:append_layer()
      local layer_fill = self.owner.layer_groups.fill:append_layer()
      self.layers.line = layer_line
      self.layers.fill = layer_fill
      do
         layer_line.x = x_min * 32
         layer_line.y = y_min * 32
         layer_line.data = self.images.line
         --
         layer_fill.x = x_min * 32
         layer_fill.y = y_min * 32
         layer_fill.data = self.images.fill
      end
   end
   function CellOutlineIsland:draw()
      local x_min = self.bounds.x[1] - 1
      local x_max = self.bounds.x[2] + 1
      local y_min = self.bounds.y[1] - 1
      local y_max = self.bounds.y[2] + 1
      --
      local img_line = self.images.line
      local img_fill = self.images.fill
      if not (img_line and img_fill) then
         error("CellOutlineIsland:draw must be called only after CellOutlineIsland:crete_canvas_data")
      end
      local color = self.owner.color
      self:set_visible(false)
      --
      for x = x_min, x_max do
         local col = self.cells[x]
         if col then
            col_p = self.cells[x - 1]
            col_n = self.cells[x + 1]
            --
            local x_prior = (x - x_min) * 32 - 1 -- pixel coordinates
            local x_cell  = x_prior +  1 -- pixel coordinates
            local x_after = x_cell  + 32 -- pixel coordinates
            --
            for y = y_min, y_max do
               local cell = self.cells[x][y]
               if cell then
                  local y_prior = (y - y_min) * 32 - 1 -- pixel coordinates
                  local y_cell  = y_prior +  1 -- pixel coordinates
                  local y_after = y_cell  + 32 -- pixel coordinates
                  --
                  img_fill:draw_rect({
                     x = x_cell,
                     y = y_cell,
                     w = 32,
                     h = 32,
                     fill_color = color
                  })
                  --
                  if y > 1 then -- upper corners
                     if not col_p or not col_p[y - 1] then
                        img_line:set_pixel(x_prior, y_prior, color) -- upper-left
                     end
                     if not col_n or not col_n[y - 1] then
                        img_line:set_pixel(x_after, y_prior, color) -- upper-right
                     end
                  end
                  if y < y_max then -- lower corners
                     if not col_p or not col_p[y + 1] then
                        img_line:set_pixel(x_prior, y_after, color) -- lower-left
                     end
                     if not col_n or not col_n[y + 1] then
                        img_line:set_pixel(x_after, y_after, color) -- lower-right
                     end
                  end
                  if not col_p or not col_p[y] then -- left
                     img_line:draw_rect({
                        fill_color = color,
                        w =  1,
                        h = 32,
                        x = x_prior,
                        y = y_cell,
                     })
                  end
                  if not col_n or not col_n[y] then -- right
                     img_line:draw_rect({
                        fill_color = color,
                        w =  1,
                        h = 32,
                        x = x_after,
                        y = y_cell,
                     })
                  end
                  if y > 1 and not col[y - 1] then -- above
                     img_line:draw_rect({
                        fill_color = color,
                        w = 32,
                        h =  1,
                        x = x_cell,
                        y = y_prior,
                     })
                  end
                  if y < y_max and not col[y + 1] then -- below
                     img_line:draw_rect({
                        fill_color = color,
                        w = 32,
                        h =  1,
                        x = x_cell,
                        y = y_after,
                     })
                  end
               end
            end
         end
      end
      --
      self:set_visible(true)
   end
   function CellOutlineIsland:has_canvas_data()
      if not (self.layers.fill and self.layers.line) then
         return false
      end
      if not (self.images.fill and self.images.line) then
         return false
      end
      return true
   end
   function CellOutlineIsland:merge(other)
      if not other or other == self then
         return
      end
      local ab = self.bounds
      local bb = other.bounds
      do
         local ab = ab.x
         local bb = bb.x
         ab[1] = math.min(ab[1], bb[1])
         ab[2] = math.max(ab[2], bb[2])
      end
      do
         local ab = ab.y
         local bb = bb.y
         ab[1] = math.min(ab[1], bb[1])
         ab[2] = math.max(ab[2], bb[2])
      end
      for x = bb.x[1], bb.x[2] do
         local col_a = self.cells[x]
         local col_b = other.cells[x]
         if col_b then
            if not col_a then
               col_a = {}
               self.cells[x] = col_a
            end
            for y = bb.y[1], bb.y[2] do
               if col_b[y] then
                  col_a[y] = true
               end
            end
         end
      end
   end
   function CellOutlineIsland:set_visible(state)
      self.layers.line.visible = state
      self.layers.fill.visible = state
   end
end

local CellOutlineMap = {}
CellOutlineMap.__index = CellOutlineMap
do
   function CellOutlineMap:new(filename)
      if not filename then
         error("CellOutlineMap:new must be passed a filename")
      end
      local instance = setmetatable({}, self)
      instance.filename = filename
      instance.color    = "#888"
      instance.cells    = {} -- cleared after islands are generated
      instance.bounds   = { x = { false, false }, y = { false, false } }
      instance.islands  = {}
      instance.layer_groups = {
         line = false,
         fill = false,
      }
      return instance
   end
   function CellOutlineMap:accept_cell(x, y)
      local col = self.cells[x]
      if not col then
         col = {}
         self.cells[x] = col
      end
      col[y] = true
      do
         local span = self.bounds.x
         if not span[1] or x < span[1] then
            span[1] = x
         end
         if not span[2] or x > span[2] then
            span[2] = x
         end
      end
      do
         local span = self.bounds.y
         if not span[1] or y < span[1] then
            span[1] = y
         end
         if not span[2] or y > span[2] then
            span[2] = y
         end
      end
   end
   function CellOutlineMap:create_canvas_data(root_line_group, root_fill_group)
      local lg = self.layer_groups
      if lg.line or lg.fill then
         error("This CellOutlineMap already has canvas data.")
      end
      local group_line = root_line_group:append_layer_group()
      local group_fill = root_fill_group:append_layer_group()
      group_line.name = "Cells altered by " .. self.filename .. " (outlines)"
      group_fill.name = "Cells altered by " .. self.filename .. " (fill)"
      lg.line = group_line
      lg.fill = group_fill
   end
   function CellOutlineMap:generate_islands()
      if not self.bounds.x[1] or not self.bounds.y[1] then -- skip island generation if this map is empty
         self.cells = nil
         return
      end
      local count = 0
      local dummy = {}
      for x = self.bounds.x[1], self.bounds.x[2] do
         self.cells[x - 2] = nil -- free past columns as soon as possible, so they can be GC'd sooner
         --
         local col = self.cells[x]
         if col then
            local x_prev = self.cells[x - 1] or dummy
            for y = self.bounds.y[1], self.bounds.y[2] do
               local cell = col[y]
               if cell then
                  --
                  -- We want to find out what island the cell belongs to (or create an island), 
                  -- and store that island's index in place of the cell's boolean "true" value.
                  --
                  local a = tonumber(x_prev[y])
                  local b = tonumber(col[y - 1])
                  local island = nil
                  do
                     --
                     -- Consider the case where the current cell happens to be adjacent to two 
                     -- different islands, one above and one to the left. Not only do we need 
                     -- to merge these islands; we need to remember that we've merged them, 
                     -- because a future cell may be adjacent to either of the merged islands.
                     --
                     -- Merging the data for the two islands is simple enough; remembering the 
                     -- merge is more challenging. What we'll do is, when we're merging two 
                     -- islands self.islands[a] and self.islands[b], we'll set self.islands[a] 
                     -- to b -- that is, to the index of the island it was merged with.
                     --
                     -- This, of course, means that self.islands[n] can be an island object or 
                     -- the numeric index of some other island object in the same list.
                     --
                     local ia = self.islands[a or dummy]
                     local ib = self.islands[b or dummy]
                     island = ia or ib
                     while tonumber(island) do -- a loop is needed because a "chain" of merged islands can come to exist
                        a = island -- for below, when we overwrite the "true" value with an island index: we want to use the "most recent" index of this island
                        island = self.islands[island]
                     end
                     if ia and ib and ia ~= ib and not tonumber(ia) and not tonumber(ib) then
                        ia:merge(ib)
                        self.islands[b] = a
                     end
                  end
                  --
                  -- If this cell is adjacent to an already-existing island, we now know which 
                  -- island that is; alternatively, we know that the cell is not adjacent to an 
                  -- already-existing island, which means we must create one.
                  --
                  if island then
                     col[y] = a or b
                  else
                     count = count + 1
                     col[y] = count
                     --
                     island = CellOutlineIsland:new(self)
                     self.islands[count] = island
                  end
                  island:accept_cell(x, y)
               end
            end
         end
      end
      self.cells = nil
      --
      -- There's one last item of business. If islands were merged above, then 
      -- self.islands is a mixed array of island objects and integers. We want 
      -- to retain only the objects.
      --
      local pruned = {}
      local j = 1
      for i = 1, count do
         local island = self.islands[i]
         if island and not tonumber(island) then
            pruned[j] = island
            j = j + 1
         end
      end
      self.islands = pruned
   end
   function CellOutlineMap:has_any_islands()
      if self.cells then
         return nil -- islands not yet generated
      end
      return #self.islands > 0
   end
   function CellOutlineMap:set_color(c)
      self.color = c
      for i = 1, #self.islands do
         local island = self.islands[i]
         if island:has_canvas_data() then
            island:draw()
         end
      end
   end
   function CellOutlineMap:set_visible(state)
      self.layer_groups.line.visible = state
      self.layer_groups.fill.visible = state
   end
end

local CellOutlineMapAllFiles = {}
CellOutlineMapAllFiles.__index = CellOutlineMapAllFiles
do
   function CellOutlineMapAllFiles:new()
      local instance = setmetatable({}, self)
      instance.maps = {}
      return instance
   end
   function CellOutlineMapAllFiles:accept(filename, x, y)
      local map = self:get_or_create_file(filename)
      map:accept_cell(x, y)
   end
   function CellOutlineMapAllFiles:for_each_map(functor)
      local t    = self.maps
      local k, v = next(t)
      while k do
         functor(v)
         k, v = next(t, k)
      end
   end
   function CellOutlineMapAllFiles:get_filename_list()
      local list = {}
      local i    = 1
      local t    = self.maps
      local k, v = next(t)
      while k do
         list[i] = k
         i = i + 1
         k, v = next(t, k)
      end
      return list
   end
   function CellOutlineMapAllFiles:get_or_create_file(filename)
      local map = self.maps[filename]
      if not map then
         map = CellOutlineMap:new(filename)
         self.maps[filename] = map
      end
      return map
   end
   function CellOutlineMapAllFiles:get_map(filename)
      return self.maps[filename]
   end
   function CellOutlineMapAllFiles:has_file(filename)
      return self.maps[filename] and true
   end
end

---

local render_worldspace_height = nil
do
   local canvas   = HeightmapWindow.controls.canvas
   local progress = HeightmapWindow.controls.progress
   function _update_progress(text, min, max, value)
      local p = HeightmapWindow.controls.progress
      if text then
         p.format = text
      end
      if min then
         p.minimum = min
      end
      if max then
         p.maximum = max
      end
      if value then
         p.value = value
      end
   end
   
   local QUAD_BOUNDS = {
      top_left = {
         vx = { min =  2, max = 17 },
         vy = { min = 18, max = 32 },
      },
      top_right = {
         vx = { min = 18, max = 33 },
         vy = { min = 18, max = 32 },
      },
      bottom_left = {
         vx = { min =  2, max = 17 },
         vy = { min =  1, max = 17 },
      },
      bottom_right = {
         vx = { min = 18, max = 33 },
         vy = { min =  1, max = 17 },
      },
   }

   render_worldspace_height = function()
      local world = nil
      local world_info = nil
      do
         world = HeightmapWindow.controls.options.world.form
         if not world then
            dovah.log_message("No worldspace to work with!")
            return
         end
         world_info = {
            heights = {
               land  = world.default_land_height,
               water = world.default_water_height,
            },
         }
      end
      
      local world_first_file = nil -- we shouldn't outline cells defined in the same file as the worldspace, because that'll be nearly all of them
      local world_filenames  = {}
      do
         local list = world:get_source_file_list()
         local file = list[1]
         if file then
            world_first_file = file.filename
         end
         for i = 2, #list do
            world_filenames[i - 1] = list[i].filename
         end
      end
      local all_maps = CellOutlineMapAllFiles:new()
      for i = 1, #world_filenames do
         all_maps:get_or_create_file(world_filenames[i])
      end
      
      local cells   = world:get_all_cells()
      local count   = #cells
      local extents = nil
      local rasters = nil
      
      _update_progress("Checking world height range...", 0, count, 0)
      do
         function _update(t, v, b) -- call _update(tbl, v) or _update(tbl, min, max)
            if not t.min or v < t.min then
               t.min = v
            end
            if b then
               if not t.max or b > t.max then
                  t.max = b
               end
            else
               if not t.max or v > t.max then
                  t.max = v
               end
            end
         end
         --
         extents = {
            x = {
               min = nil,
               max = nil,
            },
            y = {
               min = nil,
               max = nil,
            },
            z = {
               min = nil,
               max = nil,
            },
            width  = nil,
            height = nil,
         }
         --
         local z_min = nil
         local z_max = nil
         for i = 1, count do
            local cell = cells[i]
            local land = cell.landscape
            if land then
               local min = land:get_minimum_height()
               local max = land:get_maximum_height()
               _update(extents.z, min, max)
            end
            local gc = cell.grid_coords
            local gx = gc.x
            local gy = -gc.y
            _update(extents.x, gx)
            _update(extents.y, gy)
            --
            progress.value = i
         end
         --
         if not extents.x.min then
            dovah.log_message("Unable to find any cells in this worldspace!")
            return
         end
         if not extents.z.min then
            dovah.log_message("Unable to find any landscape data in this worldspace!")
            return
         end
         --
         extents.width  = (extents.x.max - extents.x.min) + 1
         extents.height = (extents.y.max - extents.y.min) + 1
         --
         extents.z.span = extents.z.max - extents.z.min
         if extents.z.span == 0 then
            extents.z.span = 1
         end
      end
      
      _update_progress("Preparing canvas...", 0, 0, 0)
      do
         rasters = {
            render  = {},
            working = {},
         }
         --
         local IMAGE_W = 32 * extents.width
         local IMAGE_H = 32 * extents.height
         canvas.width  = IMAGE_W
         canvas.height = IMAGE_H
         dovah.log_message("Canvas size: %dx%dpx", IMAGE_W, IMAGE_H)
         do
            local lowest = nil
            for i = 1, #LAYER_SPEC do
               local spec = LAYER_SPEC[i]
               local name = spec.name
               local show = HeightmapWindow.state.layer_visibility[name]
               --
               rasters.working[name] = raster.new({
                  width  = 32,
                  height = 32,
                  background_color = TRANSPARENT,
               })
               --
               rasters.render[name] = raster.new({
                  width  = IMAGE_W,
                  height = IMAGE_H,
                  background_color = TRANSPARENT,
               })
               local layer = canvas:append_layer()
               layer.data = rasters.render[name]
               layer.name = name
               HeightmapWindow.state.layers[name] = layer
               if lowest then
                  if spec.blend_mode then
                     layer.blend_mode = spec.blend_mode
                  end
                  if spec.opacity then
                     layer.opacity = spec.opacity
                  end
               elseif show then
                  lowest = layer
               end
               if not show then
                  layer.visible = false
               end
            end
         end
      end
      
      _update_progress("Drawing cells...", 0, count, 0)
      do
         local rw = rasters.working
         local rr = rasters.render
         
         function _alpha_from_height(land, water)
            local diff = water - land
            if diff > 0 then
               local span  = 1 - WATER_MIN_ALPHA
               local alpha = (math.min(1, diff / WATER_DEPTH) * span) + WATER_MIN_ALPHA
               alpha = math.min(1, alpha)
               return math.ceil(alpha * 255)
            end
            return 0
         end
         function _do_grid_flip(x, y)
            return x, (33 - y + 1)
         end
         function _for_each_quad(land, functor)
            local quads = land.quads
            functor("bottom_left",  quads.bottom_left)
            functor("bottom_right", quads.bottom_right)
            functor("top_left",     quads.top_left)
            functor("top_right",    quads.top_right)
         end
         
         local benchmark = dovah.benchmark_start()
         for i = 1, count do
            local cell = cells[i]
            --
            local water_height = nil
            do
               local a = cell.water_height -- nil if we're defaulting to the world water height
               local b = world_info.heights.water
               water_height = a or b or -9999999
            end
            --
            local x = nil
            local y = nil
            do
               local gc = cell.grid_coords
               x  =  gc.x - extents.x.min
               y  = -gc.y - extents.y.min -- invert Y so that north is up
               x = x * 32
               y = y * 32
            end
            --
            do -- File list
               local files = cell:get_source_file_list()
               for j = 1, #files do
                  local file = files[j]
                  if file then
                     local name = file.filename
                     if name ~= world_first_file then
                        all_maps:accept(name, x / 32, y / 32) -- this function wants (north = up) grid coordinates, not pixel coordinates
                     end
                  end
               end
            end
            if TEST_FOR_PERSISTENT_REFS then
               --
               -- This code exists to handle a theoretical edge-case. It's not known 
               -- whether the Creation Kit would ever produce this edge-case, and the 
               -- code hasn't been useful when testing Dawnguard.esm.
               --
               -- It's possible for a file to modify a cell without actually ending up 
               -- in the cell's source file list. If the cell is an exterior cell, and 
               -- the file places a persistent reference inside of the cell, then that 
               -- persistent reference won't actually be stored as part of the cell. 
               -- Instead, the reference goes inside of what's called a "persistent 
               -- cell," which belongs to the worldspace as a whole.
               --
               -- If every normal cell is a box that encloses a small chunk of space 
               -- within the worldspace, then you can think of the "persistent cell" 
               -- as a large box that overlaps with the entire worldspace.
               --
               -- Of course, when DovahKit's backend loads a file, it "reparents" all 
               -- children of the persistent cell -- that is, it "moves" them so that 
               -- they belong to a "real" cell, based on their positions. This is the 
               -- same thing that the game does while loading.
               --
               -- As such, the only way to TRULY know what files modify a cell -- not 
               -- a cell form, but the physical region that we think of as a "cell" -- 
               -- is to check the cell's source file list, and then loop over all of 
               -- its persistent references.
               --
               local refs  = cell:get_all_persistent_refs()
               local count = #refs
               for i = 1, count do
                  local files = refs[i]:get_source_file_list()
                  for j = 1, #files do
                     local name = files[j].filename
                     if name ~= world_first_file then
                        all_maps:accept(name, x / 32, y / 32) -- this function wants (north = up) grid coordinates, not pixel coordinates
                     end
                  end
               end
            end
            --
            local land = cell.landscape
            if land and land.enable_vertex_heights then
               rw.water:fill(TRANSPARENT)
               --
               do
                  for u = 2, 33 do -- leftmost col overlaps with western cell, so skip it
                     for v = 1, 32 do -- bottom row overlaps with southern cell, so skip it
                        local vcolor = land:get_color_at(u, v)
                        local height = land:get_height_at(u, v)
                        local shade  = (height - extents.z.min) / extents.z.span
                        shade = math.floor(shade * 255) -- TODO: round
                        --
                        local a, b = _do_grid_flip(u, v)
                        a = a - 1 -- account for skipped col
                        b = b - 1 -- account for skipped row
                        rw.height:set_pixel(a, b, { r = shade, g = shade, b = shade })
                        rw.color:set_pixel(a, b, vcolor)
                        --
                        if cell.has_water then -- water
                           local alpha = _alpha_from_height(height, water_height)
                           if alpha > 0 then
                              rw.water:set_pixel(a, b, { r = 80, g = 160, b = 255, a = alpha })
                           end
                        end
                     end
                  end
               end
               --
               rasters.working.paint:fill(TRANSPARENT)
               local quads = land.quads
               local _draw_quad = false
               do
                  _draw_quad = function(which, quad)
                     local vx = QUAD_BOUNDS[which].vx
                     local vy = QUAD_BOUNDS[which].vy
                     --
                     do
                        local ltex  = quad.default_texture
                        local color = nil
                        if ltex then
                           color = TextureManager:get_color(ltex)
                        else
                           color = TextureManager:get_default_color()
                        end
                        color.a  = 255 -- force alpha
                        color[4] = 255 -- force alpha
                        local a, b = _do_grid_flip(vx.min, vy.min)
                        local c, d = _do_grid_flip(vx.max, vy.max)
                        a = a - 1
                        d = d - 1
                        rw.paint:draw_rect({
                           from = { a, d },
                           to   = { c, b },
                           fill_color = color,
                        })
                     end
                     quad:for_each_alpha_layer(function(layer)
                        local ltex  = layer.texture
                        local color = nil
                        if ltex then
                           color = TextureManager:get_color(ltex)
                        else
                           -- YES, blend layers can be null and therefore use the default texture too
                           color = TextureManager:get_default_color()
                        end
                        for u = vx.min, vx.max do
                           for v = vy.min, vy.max do
                              local opacity = layer:get_opacity_at_cell_position(u, v)
                              if opacity > 0 then
                                 local a, b = _do_grid_flip(u, v)
                                 a = a - 1 -- account for skipped col
                                 b = b - 1 -- account for skipped row
                                 color.a  = math.max(0, math.min(255, math.ceil(opacity * 255))) -- force alpha
                                 color[4] = color.a -- force alpha
                                 rw.paint:blend_pixel(a, b, color) -- set_pixel overwrites; blend_pixel blends
                              end
                           end
                        end
                     end)
                  end
               end
               _for_each_quad(land, _draw_quad)
               --
               rr.height:draw_raster(rw.height, x + 1, y + 1)
               rr.color:draw_raster (rw.color,  x + 1, y + 1)
               rr.water:draw_raster (rw.water,  x + 1, y + 1)
               rr.paint:draw_raster (rw.paint,  x + 1, y + 1)
            else
               --
               -- The Creation Kit won't always encode a valid heightmap for a cell if the 
               -- cell's terrain exactly matches the worldspace's default land height -- 
               -- including the portions that overlap adjacent cells.
               --
               -- As of this writing, DovahKit can't remember whether a cell had a heightmap 
               -- or not, post-load, but it *can* remember whether the landscape was *flagged* 
               -- as having a heightmap, which is what we test here.
               --
               local height = world_info.heights.land
               local shade  = math.max(0, math.min(1, (height - extents.z.min) / extents.z.span))
               shade = math.floor(shade * 255) -- TODO: round
               --
               rw.height:fill({ r = shade, g = shade, b = shade })
               rr.height:draw_raster(rw.height, x + 1, y + 1)
               --
               local color = TextureManager:get_default_color()
               color.a  = 255 -- force alpha
               color[4] = 255 -- force alpha
               rw.paint:fill(color)
               rr.paint:draw_raster(rw.paint, x + 1, y + 1)
               --
               if cell.has_water and height < water_height then -- water
                  local alpha = _alpha_from_height(height, water_height)
                  if alpha > 0 then
                     rw.water:fill({ r = 80, g = 160, b = 255, a = alpha })
                     rr.water:draw_raster(rw.water,  x + 1, y + 1)
                  end
               end
            end
            progress.value = i
         end
         dovah.benchmark_stop(benchmark)
         dovah.log_message("Time taken for draw: %s milliseconds (%s microseconds)", benchmark:milliseconds(), benchmark:microseconds())
         
         benchmark = dovah.benchmark_start()
         do -- Cell outlines
            local files = all_maps:get_filename_list()
            local count = #files
            _update_progress("Drawing outlines...", 0, count, 0)
            --
            local HUE_PER_FILE = math.ceil(360 / count)
            --
            local root_group = false
            local line_group = false
            local fill_group = false
            for i = 1, count do
               local name = files[i]
               local map  = all_maps.maps[name]
               map:set_color(string.format("hsl(%sdeg, 100%%, 50%%)", HUE_PER_FILE * i))
               map:generate_islands()
               --
               local islands = map.islands
               if #islands > 0 then
                  if not root_group then -- lazy-create these layer groups
                     root_group = canvas:append_layer_group()
                     root_group.name = "Cells by file"
                     fill_group = root_group:append_layer_group()
                     fill_group.name = "Fill"
                     line_group = root_group:append_layer_group()
                     line_group.name = "Outlines"
                     --
                     fill_group.opacity = math.max(0, math.min(1, CELL_OUTLINE_FILL_OPACITY / 255))
                  end
                  map:create_canvas_data(line_group, fill_group)
                  for j = 1, #islands do
                     local island = islands[j]
                     island:create_canvas_data()
                     island:draw()
                  end
               end
               progress.value = i
            end
         end
         dovah.benchmark_stop(benchmark)
         dovah.log_message("Time taken for islands: %s milliseconds (%s microseconds)", benchmark:milliseconds(), benchmark:microseconds())
         HeightmapWindow:import_cell_outline_data(all_maps)
         _update_progress("Done!", 0, count, count)
      end
   end
end

HeightmapWindow.controls.options.execute:on("OnActivated", "render", function()
   HeightmapWindow:set_is_locked(true)
   HeightmapWindow:clear_canvas()
   HeightmapWindow:clear_cell_outline_toggles()
   render_worldspace_height()
   HeightmapWindow:set_is_locked(false)
end)
HeightmapWindow:show()
