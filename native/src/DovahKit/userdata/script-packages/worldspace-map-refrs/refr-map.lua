RefrMap = {}
RefrMap.__index = RefrMap
do
   function RefrMap:new()
      local instance = setmetatable({}, self)
      instance.maps = {}
      return instance
   end
   function RefrMap:accept(base_form, x, y)
      local map = self:get_map(base_form)
      if map then
         map:accept_ref(x, y)
      end
   end
   function RefrMap:for_each_map(functor)
      local t    = self.maps
      local k, v = next(t)
      while k do
         functor(v)
         k, v = next(t, k)
      end
   end
   function RefrMap:get_base_form_list()
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
   function RefrMap:get_or_create_map(base_form)
      local map = self.maps[base_form]
      if not map then
         map = CellOutlineMap:new(base_form)
         self.maps[base_form] = map
      end
      return map
   end
   function RefrMap:get_map(base_form)
      return self.maps[base_form]
   end
   function RefrMap:has_map(base_form)
      return self.maps[base_form] and true
   end
end