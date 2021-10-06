local dummy_progress = ui.progress_bar.new()

World = {}
World.__index = World
do
   function World:new(world)
      local instance = setmetatable({}, self)
      instance.world      = world
      instance.cells      = false
      instance.cell_count = 0
      instance.extents    = false
      instance.reference_by_form_type_cache = false
      do
         self.first_filename = ""
         local list = world:get_source_file_list()
         local file = list[1]
         if file then
            self.first_filename = file.filename
         end
      end
      return instance
   end
   function World:map_base_form_refs(base_forms, progress, allow_caching)
      if #base_forms <= 0 then
         return
      end
      if not progress then
         progress = dummy_progress
      end
      local extents = self:get_extents()
      local cells   = self:get_cells()
      local count   = self.cell_count
      if count <= 0 then
         dovah.log_message("Unable to find any cells in this worldspace!")
         return
      end
      --
      local all_maps      = RefrMap:new()
      local base_form_set = {}
      for i = 1, #base_forms do
         local form = base_forms[i]
         if form then
            base_form_set[form] = true
            all_maps:get_or_create_map(form)
         end
      end
      --
      if self.reference_by_form_type_cache then
         local cache_root = self.reference_by_form_type_cache
         local want_types = {}
         for i = 1, #base_forms do
            local base = base_forms[i]
            local ft   = base.form_type
            want_types[ft] = true
         end
         progress.minimum = 0
         local benchmark = dovah.benchmark_start()
         local k, v = next(want_types)
         while k do
            local list = cache_root[k]
            if list then
               progress.maximum = #list
               progress.value   = 0
               progress.format  = "Searching cached forms of type: " .. k.signature .. "..."
               for i = 1, #list do
                  local form = list[i]
                  local base = form.base_form
                  if base_form_set[base] then
                     local p = form.position
                     local x = p.x - (extents.x.min * 4096)
                     local y = -p.y - (extents.y.min * 4096)
                     all_maps:accept(base, x, y)
                  end
                  progress.value = i
               end
            end
            k, v = next(want_types, k)
         end
         dovah.benchmark_stop(benchmark)
         progress.maximum = 1
         progress.value   = 1
         return {
            used_cache = true,
            maps       = all_maps,
            benchmark  = benchmark,
         }
      end
      progress.minimum = 0
      progress.maximum = count
      progress.value   = 0
      local benchmark = false
      if allow_caching then
         local cache_root = {}
         self.reference_by_form_type_cache = cache_root
         benchmark = dovah.benchmark_start()
         for i = 1, count do
            local cell = cells[i]
            local refs = cell:get_all_refs()
            for i = 1, #refs do
               local form = refs[i]
               if not form.disabled then
                  local base = form.base_form
                  if base then
                     local ft = base.form_type
                     do
                        local list = cache_root[ft]
                        if not list then
                           list = {}
                           cache_root[ft] = list
                        end
                        list[#list + 1] = form
                     end
                     if base_form_set[base] then
                        local p = form.position
                        local x = p.x - (extents.x.min * 4096)
                        local y = -p.y - (extents.y.min * 4096)
                        all_maps:accept(base, x, y)
                     end
                  end
               end
            end
            progress.value = i
         end
         dovah.benchmark_stop(benchmark)
      else
         benchmark = dovah.benchmark_start()
         for i = 1, count do
            local cell = cells[i]
            local refs = cell:get_all_refs()
            for i = 1, #refs do
               local form = refs[i]
               if not form.disabled then
                  local base = form.base_form
                  if base_form_set[base] then
                     local p = form.position
                     local x = p.x - (extents.x.min * 4096)
                     local y = -p.y - (extents.y.min * 4096)
                     all_maps:accept(base, x, y)
                  end
               end
            end
            progress.value = i
         end
         dovah.benchmark_stop(benchmark)
      end
      progress.value = count
      return {
         used_cache = false,
         maps       = all_maps,
         benchmark  = benchmark,
      }
   end
   function World:get_cells()
      if self.cells then
         return self.cells
      end
      local cells = self.world:get_all_cells()
      self.cells      = cells
      self.cell_count = #cells
      return cells
   end
   function World:get_extents()
      if self.extents then
         return self.extents
      end
      local world   = self.world
      local cells   = self:get_cells()
      local count   = self.cell_count
      local extents = {
         x = Range:new(),
         y = Range:new(),
         z = Range:new(),
         width  = nil,
         height = nil,
         default_heights = {
            land  = world.default_land_height,
            water = world.default_water_height,
         },
      }
      for i = 1, count do
         local cell = cells[i]
         local gc   = cell.grid_coords
         local gx   = gc.x
         local gy   = -gc.y
         extents.x:accept(gx)
         extents.y:accept(gy)
      end
      if not extents.x.min then
         self.extents = extents
         return
      end
      --
      extents.width  = (extents.x.max - extents.x.min) + 1
      extents.height = (extents.y.max - extents.y.min) + 1
      if extents.z.max then
         extents.z.span = extents.z.max - extents.z.min
         if extents.z.span == 0 then
            extents.z.span = 1
         end
      else
         extents.z.span = nil
      end
      self.extents = extents
      return extents
   end
end

WorldMapper = {}
WorldMapper.__index = WorldMapper
do
   WorldMapper.worlds = {}
   function WorldMapper:find_all_in(world, base_forms, canvas, progress, allow_caching)
      assert(world ~= nil, "you must supply a worldspace form")
      assert(form_types.worldspace.is(world), "what you supplied was not a worldspace form")
      assert(ui.canvas.is(canvas), "you must supply a canvas")
      if not progress then
         progress = dummy_progress
      end
      if allow_caching == nil then
         allow_caching = true
      end
      local wrapped = self:get_world(world)
      local extents = wrapped:get_extents()
      do -- Clear canvas.
         local layers = {}
         local native = canvas.layers
         for i = 1, #native do
            layers[i] = native[i]
         end
         for i = 1, #layers do
            canvas:remove_layer(layers[i])
         end
      end
      do -- Get pre-rendered world terrain image.
         progress.minimum = 0
         progress.maximum = 0
         progress.value   = 0
         progress.format  = "Preparing canvas..."
         --
         local IMAGE_W = 32 * extents.width
         local IMAGE_H = 32 * extents.height
         canvas.width  = IMAGE_W
         canvas.height = IMAGE_H
         dovah.log_message("Canvas size: %dx%dpx", IMAGE_W, IMAGE_H)
         do
            local name  = string.format("pre-rendered/%s-%s.png", world.editor_id, wrapped.first_filename or "nil")
            local image = dovah.package.load_file({ path = name, type = "png" })
            if image then
               local layer = canvas:append_layer()
               layer.data = image
               layer.name = "Pre-rendered terrain"
               if image.width ~= IMAGE_W or image.height ~= IMAGE_H then
                  warn("World size doesn't match the prerendered terrain image!")
               end
            else
               dovah.log_message("Failed to load prerendered terrain for this worldspace from file:\n%s", name)
            end
         end
      end
      progress.minimum = 0
      progress.format  = "Finding references..."
      local results = wrapped:map_base_form_refs(base_forms, progress, allow_caching)
      dovah.log_message("Time taken for search: %s milliseconds (%u microseconds)\n - Search %s cached data.",
         results.benchmark:milliseconds(),
         results.benchmark:microseconds(),
         results.used_cache and "used" or "did not use"
      )
      --
      -- Render results:
      --
      progress.minimum = 0
      progress.maximum = #base_forms
      progress.value   = 0
      progress.format  = "Drawing results..."
      local benchmark = dovah.benchmark_start()
      do
         local root_group = false
         local line_group = false
         local fill_group = false
         local all_maps   = results.maps
         for i = 1, #base_forms do
            local name = base_forms[i]
            local map  = all_maps.maps[name]
            map:generate_islands()
            --
            local islands = map.islands
            if #islands > 0 then
               if not root_group then -- lazy-create these layer groups
                  root_group = canvas:append_layer_group()
                  root_group.name = "Base forms"
                  line_group = root_group:append_layer_group()
                  line_group.name = "Outlines"
                  fill_group = root_group:append_layer_group()
                  fill_group.name = "Fill"
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
      progress.value  = #base_forms
      progress.format = "Done!"
      dovah.log_message("Time taken for drawing results: %s milliseconds (%s microseconds)", benchmark:milliseconds(), benchmark:microseconds())
      --
      return results.maps
   end
   function WorldMapper:forget_world(world)
      assert(world ~= nil, "you must supply a worldspace form")
      assert(form_types.worldspace.is(world), "what you supplied was not a worldspace form")
      local list  = self.worlds
      local size  = #list
      local index = nil
      for i = 1, size do
         local item = list[i]
         if item.world == world then
            index = i
            break
         end
      end
      if not index then
         return
      end
      list[index] = list[index + 1]
      for i = index + 1, size do
         list[i] = list[i + 1]
      end
   end
   function WorldMapper:get_world(world)
      assert(world ~= nil, "you must supply a worldspace form")
      assert(form_types.worldspace.is(world), "what you supplied was not a worldspace form")
      local list = self.worlds
      local size = #list
      for i = 1, size do
         local item = list[i]
         if item.world == world then
            return item
         end
      end
      local item = World:new(world)
      list[size + 1] = item
      return item
   end
end