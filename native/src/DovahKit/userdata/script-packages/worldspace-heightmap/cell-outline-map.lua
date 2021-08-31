CellOutlineMap = {}
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