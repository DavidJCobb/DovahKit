Range = {}
do
   Range.__index = Range
   function Range:new()
      local instance = setmetatable({}, self)
      instance.min = nil
      instance.max = nil
      return instance
   end
   function Range:accept(v)
      if not self.min then
         self.min = v
         self.max = v
         return
      end
      self.min = math.min(self.min, v)
      self.max = math.max(self.max, v)
   end
   function Range:empty()
      return not (self.min and self.max)
   end
   function Range:merge(a, b)
      if not b then
         if type(a) ~= "table" then
            error("expected two numbers or one Range")
         end
         b = a.max
         a = a.min
      end
      if not self.min then
         self.min = a
         self.max = b
         return
      end
      self.min = math.min(self.min, a)
      self.max = math.max(self.max, b)
   end
end