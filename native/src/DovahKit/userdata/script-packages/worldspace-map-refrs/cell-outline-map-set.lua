CellOutlineMapAllFiles = {}
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