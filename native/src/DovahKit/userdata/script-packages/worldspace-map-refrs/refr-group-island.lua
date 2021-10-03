RefrGroupIsland = {}
RefrGroupIsland.__index = RefrGroupIsland
do
   local LAND_VERTICES_PER_CELL      <const> = 32
   local WORLD_UNITS_PER_CELL_SIDE   <const> = 4096
   local WORLD_UNITS_PER_LAND_VERTEX <const> = (WORLD_UNITS_PER_CELL_SIDE / LAND_VERTICES_PER_CELL)
   
   local FILL_RADIUS   <const> = 5
   local LINE_RADIUS   <const> = FILL_RADIUS + 2
   local LAYER_PADDING <const> = LINE_RADIUS * 2

   function RefrGroupIsland:new(owner)
      if not owner then
         error("RefrGroupIsland instances must be owned by a RefrGroup")
      end
      local instance = setmetatable({}, self)
      instance.cells  = {} -- used for merging, etc.
      instance.owner  = owner
      instance.bounds = { x = Range:new(), y = Range:new() }
      instance.layers = {
         fill = false,
         line = false,
      }
      instance.images = {
         fill = false,
         line = false,
      }
      instance.paths = {
         fill = false,
         line = false,
      }
      instance.refs = {} -- list of world-relative coordinate tuples
      return instance
   end
   function RefrGroupIsland:accept_cell(cell)
      if #cell.refs < 1 then
         return
      end
      --
      local min_x = cell.refs[1].x
      local max_x = min_x
      local min_y = cell.refs[1].y
      local max_y = min_y
      --
      local count = #self.refs
      for i = 1, #cell.refs do
         local entry = cell.refs[i]
         count = count + 1
         self.refs[count] = entry
         --
         min_x = math.min(entry.x, min_x)
         max_x = math.max(entry.x, max_x)
         min_y = math.min(entry.y, min_y)
         max_y = math.max(entry.y, max_y)
      end
      cell.refs = {}
      --
      min_x = math.floor(min_x / WORLD_UNITS_PER_CELL_SIDE)
      max_x = math.floor(max_x / WORLD_UNITS_PER_CELL_SIDE)
      min_y = math.floor(min_y / WORLD_UNITS_PER_CELL_SIDE)
      max_y = math.floor(max_y / WORLD_UNITS_PER_CELL_SIDE)
      self.bounds.x:merge(min_x, max_x)
      self.bounds.y:merge(min_y, max_y)
   end
   function RefrGroupIsland:accept_ref(world_x, world_y)
      do
         local gx = math.floor(world_x / WORLD_UNITS_PER_CELL_SIDE)
         local gy = math.floor(world_y / WORLD_UNITS_PER_CELL_SIDE)
         --
         local col = self.cells[gx]
         if not col then
            col = {}
            self.cells[gx] = col
         end
         if not col[gy] then
            col[gy] = true
            self.bounds.x:accept(gx)
            self.bounds.y:accept(gy)
         end
      end
      --
      local list = self.refs
      list[#list + 1] = { x = world_x, y = world_y }
   end
   function RefrGroupIsland:create_canvas_data()
      local x_min = self.bounds.x.min - 1
      local x_max = self.bounds.x.max + 1
      local y_min = self.bounds.y.min - 1
      local y_max = self.bounds.y.max + 1
      --
      self.images.line = raster.new({
         width  = (x_max - x_min + 1) * LAND_VERTICES_PER_CELL + LAYER_PADDING,
         height = (y_max - y_min + 1) * LAND_VERTICES_PER_CELL + LAYER_PADDING,
      })
      self.images.fill = raster.new({
         width  = (x_max - x_min + 1) * LAND_VERTICES_PER_CELL + LAYER_PADDING,
         height = (y_max - y_min + 1) * LAND_VERTICES_PER_CELL + LAYER_PADDING,
      })
      local layer_line = self.owner.layer_groups.line:append_layer()
      local layer_fill = self.owner.layer_groups.fill:append_layer()
      self.layers.line = layer_line
      self.layers.fill = layer_fill
      do
         layer_line.x = x_min * LAND_VERTICES_PER_CELL - LINE_RADIUS
         layer_line.y = y_min * LAND_VERTICES_PER_CELL - LINE_RADIUS
         layer_line.data = self.images.line
         --
         layer_fill.x = x_min * LAND_VERTICES_PER_CELL - LINE_RADIUS
         layer_fill.y = y_min * LAND_VERTICES_PER_CELL - LINE_RADIUS
         layer_fill.data = self.images.fill
      end
   end
   function RefrGroupIsland:draw(force_rebuild)
      local x_min = self.bounds.x.min - 1
      local x_max = self.bounds.x.max + 1
      local y_min = self.bounds.y.min - 1
      local y_max = self.bounds.y.max + 1
      --
      local img_line = self.images.line
      local img_fill = self.images.fill
      if not (img_line and img_fill) then
         error("RefrGroupIsland:draw must be called only after RefrGroupIsland:create_canvas_data")
      end
      local color = self.owner.color
      self:set_visible(false)
      --
      local ox = x_min * LAND_VERTICES_PER_CELL - LINE_RADIUS
      local oy = (y_min - 1) * LAND_VERTICES_PER_CELL - LINE_RADIUS
      local path_fill = self.paths.fill
      local path_line = self.paths.line
      if force_rebuild or not (path_fill and path_line) then
         path_fill = raster_draw_path.new()
         path_line = raster_draw_path.new()
         self.paths.fill = path_fill
         self.paths.line = path_line
         for i = 1, #self.refs do
            local pair = self.refs[i]
            --
            local x = pair.x / WORLD_UNITS_PER_LAND_VERTEX -- world coordinates -> heightmap coordinates
            local y = pair.y / WORLD_UNITS_PER_LAND_VERTEX -- world coordinates -> heightmap coordinates
            x = x - ox
            y = y - oy
            x = math.round(x)
            y = math.round(y)
            --
            path_fill:add_ellipse({
               center = { x, y },
               radius = FILL_RADIUS,
            })
            path_line:add_ellipse({
               center = { x, y },
               radius = LINE_RADIUS,
            })
         end
      end
      img_fill:fill("#00000000") -- clear
      img_line:fill("#00000000") -- clear
      img_fill:draw_path({ path = path_fill, fill_color = color })
      img_line:draw_path({ path = path_line, fill_color = "#000" })
      --
      self:set_visible(true)
   end
   function RefrGroupIsland:has_canvas_data()
      if not (self.layers.fill and self.layers.line) then
         return false
      end
      if not (self.images.fill and self.images.line) then
         return false
      end
      return true
   end
   function RefrGroupIsland:merge(other)
      if not other or other == self then
         return
      end
      local ab = self.bounds
      local bb = other.bounds
      if bb.x:empty() or bb.y:empty() then -- nothing to merge in
         assert((not other.refs) or (#other.refs == 0))
         return
      end
      ab.x:merge(bb.x)
      ab.y:merge(bb.y)
      for x = bb.x.min, bb.x.max do
         local col_a = self.cells[x]
         local col_b = other.cells[x]
         if col_b then
            if not col_a then
               col_a = {}
               self.cells[x] = col_a
            end
            for y = bb.y.min, bb.y.max do
               if col_b[y] then
                  col_a[y] = true
               end
            end
         end
      end
      local at = #self.refs
      for i = 1, #other.refs do
         at = at + 1
         self.refs[at] = other.refs[i]
      end
      other.refs = {}
   end
   function RefrGroupIsland:set_visible(state)
      self.layers.line.visible = state
      self.layers.fill.visible = state
   end
end