RefrGroupIsland = {}
RefrGroupIsland.__index = RefrGroupIsland
do
   function RefrGroupIsland:new(owner)
      if not owner then
         error("RefrGroupIsland instances must be owned by a RefrGroup")
      end
      local instance = setmetatable({}, self)
      instance.cells  = {} -- used for merging, etc.
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
      instances.refrs = {} -- list of world-relative coordinate tuples
      return instance
   end
   function RefrGroupIsland:accept_ref(world_x, world_y)
      do
         local x = math.floor(world_x / 4096)
         local y = math.floor(world_y / 4096)
         --
         local col = self.cells[x]
         if not col then
            col = {}
            self.cells[x] = col
         end
         if not col[y] then
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
      end
      --
      local list = self.refrs
      list[#list + 1] = { world_x, world_y }
   end
   function RefrGroupIsland:create_canvas_data()
      local x_min = self.bounds.x[1] - 1
      local x_max = self.bounds.x[2] + 1
      local y_min = self.bounds.y[1] - 1
      local y_max = self.bounds.y[2] + 1
      --
      self.image = raster.new({
         width  = (x_max - x_min + 1) * 32,
         height = (y_max - y_min + 1) * 32,
      })
      local layer = self.owner.layers:append_layer()
      self.layer.line = layer
      do
         layer.x = x_min * 32
         layer.y = y_min * 32
         layer.data = self.image
      end
   end
   function RefrGroupIsland:draw()
      local x_min = self.bounds.x[1] - 1
      local x_max = self.bounds.x[2] + 1
      local y_min = self.bounds.y[1] - 1
      local y_max = self.bounds.y[2] + 1
      --
      local image = self.image
      if not image then
         error("RefrGroupIsland:draw must be called only after RefrGroupIsland:crete_canvas_data")
      end
      local color = self.owner.color
      self:set_visible(false)
      --
      local ox = x_min * 32
      local oy = y_min * 32
      for i = 1, #self.refrs do
         local pair = self.refrs[i]
         --
         local x = pair[0] / 32 -- world coordinates -> heightmap coordinates
         local y = pair[1] / 32 -- world coordinates -> heightmap coordinates
         x = x - ox
         y = y - oy
         --
         local radius = 5
         image:draw_ellipse({
            center     = { x - (radius / 2), y - (radius / 2) },
            radius     = radius,
            fill_color = color,
         })
      end
      --
      self:set_visible(true)
   end
   function RefrGroupIsland:has_canvas_data()
      if not self.layer then
         return false
      end
      if not self.image then
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
   function RefrGroupIsland:set_visible(state)
      self.layer.visible = state
   end
end