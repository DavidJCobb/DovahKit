RefrCell = {}
RefrCell.__index = RefrCell
do
   function RefrCell:new()
      local instance = setmetatable({}, self)
      instance.refs = {}
      return instance
   end
   function RefrCell:accept_ref(wx, wy)
      local list = self.refs
      list[#list] = { x = wx, y = wy }
   end
end