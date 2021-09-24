RefrGroup = {}
RefrGroup.__index = RefrGroup
do
   function RefrGroup:new(base_form)
      if not base_form then
         error("RefrGroup:new must be passed a base form")
      end
      local instance = setmetatable({}, self)
      instance.form    = base_form
      instance.color   = "#888"
      instance.cells   = {} -- cleared after islands are generated
      instance.bounds  = { x = { false, false }, y = { false, false } }
      instance.islands = {}
      instance.layers  = false
      return instance
   end
   function RefrGroup:accept_ref(wx, wy)
      local x = wx / 32
      local y = wy / 32
      --
      local col = self.cells[x]
      if not col then
         col = {}
         self.cells[x] = col
      end
      if not col[y] then
         col[y] = {}
         --
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
      col[y][#col[y] + 1] = { wx, wy }
   end
   function RefrGroup:create_canvas_data(root_group)
      if self.layers then
         error("This RefrGroup already has canvas data.")
      end
      local group = root_group:append_layer_group()
      group.name = "References with base form: " .. self.form:form_id_to_string()
      self.layers = group
   end
   function RefrGroup:generate_islands()
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
                     if not tonumber(cell) then
                        for i = 1, #cell do
                           island:accept_ref(cell[i][0], cell[i][1])
                        end
                     end
                     col[y] = a or b
                  else
                     count = count + 1
                     col[y] = count
                     --
                     island = CellOutlineIsland:new(self)
                     self.islands[count] = island
                     if not tonumber(cell) then
                        for i = 1, #cell do
                           island:accept_ref(cell[i][0], cell[i][1])
                        end
                     end
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
   function RefrGroup:has_any_islands()
      if self.cells then
         return nil -- islands not yet generated
      end
      return #self.islands > 0
   end
   function RefrGroup:set_color(c)
      self.color = c
      for i = 1, #self.islands do
         local island = self.islands[i]
         if island:has_canvas_data() then
            island:draw()
         end
      end
   end
   function RefrGroup:set_visible(state)
      self.layers.visible = state
   end
end