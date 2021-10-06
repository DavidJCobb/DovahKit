CellOutlineMap = {}
CellOutlineMap.__index = CellOutlineMap
do
   local FORCE_SINGLE_ISLAND <const> = false

   function CellOutlineMap:new(filename)
      if not filename then
         error("CellOutlineMap:new must be passed a filename")
      end
      local instance = setmetatable({}, self)
      instance.filename = filename
      instance.color    = "#888"
      instance.cells    = {} -- cleared after islands are generated
      instance.bounds   = { x = Range:new(), y = Range:new() }
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
      self.bounds.x:accept(x)
      self.bounds.y:accept(y)
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
      if self.bounds.x:empty() then
         self.cells = nil
         return
      end
      if not self.cells then
         return
      end
      if FORCE_SINGLE_ISLAND then
         local island = false
         for x = self.bounds.x.min, self.bounds.x.max do
            self.cells[x - 1] = nil
            local col = self.cells[x]
            if col then
               for y = self.bounds.y.min, self.bounds.y.max do
                  local cell = col[y]
                  if cell then
                     if not island then
                        island = RefrGroupIsland:new(self)
                     end
                     island:accept_cell(x, y)
                  end
               end
            end
         end
         self.cells = nil
         if island then
            self.islands[1] = island
         end
         return
      end
      local function bind(context, f)
         return function(...) return f(context, ...) end
      end
      local resolve_island = bind(self, function(self, i)
         while tonumber(i) do
            i = self.islands[i]
         end
         if i then
            for j = 1, #self.islands do
               if self.islands[j] == i then
                  return i, j
               end
            end
         end
         return i
      end)
      local count = 0
      local dummy = {}
      for x = self.bounds.x.min, self.bounds.x.max do
         self.cells[x - 2] = nil -- free past columns as soon as possible, so they can be GC'd sooner
         --
         local col = self.cells[x]
         if col then
            local x_prev = self.cells[x - 1] or dummy
            for y = self.bounds.y.min, self.bounds.y.max do
               local cell = col[y]
               if cell then
                  --
                  -- We want to find out what island the cell belongs to (or create an island), 
                  -- and store that island's index in place of the cell's boolean "true" value.
                  --
                  local a = tonumber(x_prev[y])  -- left
                  local b = tonumber(col[y - 1]) -- above
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
                     local na = self.islands[a or dummy]
                     local nb = self.islands[b or dummy]
                     local ia
                     local ib
                     ia, a = resolve_island(na)
                     ib, b = resolve_island(nb)
                     island = ia or ib
                     if ia and ib and ia ~= ib then
                        ia:merge(ib)
                        self.islands[b] = a
                        b = a
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
      if not self.layer_groups.line then
         return
      end
      self.layer_groups.line.visible = state
      self.layer_groups.fill.visible = state
   end
end