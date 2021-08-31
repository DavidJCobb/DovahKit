CellOutlineIsland = {}
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