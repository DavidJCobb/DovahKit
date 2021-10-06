Range = {}
do
   Range.__index = Range
   function Range:new()
      local instance = setmetatable({}, self)
      instance.min = nil
      instance.max = nil
      return instance
   end
   function Range:accept(a, b)
      if not self.min then
         self.min = a
         self.max = b or a
         return
      end
      self.min = math.min(self.min, a)
      self.max = math.max(self.max, b or a)
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