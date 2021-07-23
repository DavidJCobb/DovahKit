---------------
-- TEST CODE --
---------------

define_class(
   "Class A",
   nil,
   { -- Methods
      get_my_name = function() return "Class A" end,
   },
   { -- Getters
      name = function(t) return "Class A: " .. tostring(t) end,
   },
   { -- Setters
      foo = function(t, v) rawset(t, "foo", v) end
   }
)

define_class(
   "Class B",
   nil,
   { -- Methods
      get_my_name = function() return "Class B" end,
      query_b = function() return true end,
   },
   { -- Getters
      name = function(t) return "Class B: " .. tostring(t) end,
      getter_5 = function(t) return 5 end,
   },
   { -- Setters
      foo = function(t, v) rawset(t, "foo", v * 2) end
   }
)


define_class(
   "Class C",
   { "Class A", "Class B" },
   { -- Methods
   },
   { -- Getters
   },
   { -- Setters
      bar = function(t, v) rawset(t, "bar", v) end
   }
)



local a = setmetatable({}, REGISTRY["Class A"])
local b = setmetatable({}, REGISTRY["Class B"])
local c = setmetatable({}, REGISTRY["Class C"])

print(a:get_my_name())
print(b:get_my_name())
print(c:get_my_name())

print(b:query_b())
print(c:query_b())

a.foo = 5
print(a.foo)

b.foo = 5
print(b.foo)

c.foo = 5
print(c.foo)

function _pcall_and_print(func)
   local result, e = pcall(func)
   if not result then
      print(e)
   end
end

_pcall_and_print(function() a.bar = 5 end)
_pcall_and_print(function() print(a.baritone) end)
_pcall_and_print(function() a:bad() end)

print("\nc pairs:")
for k, v in pairs(c) do
   print(k .. " == " .. tostring(v))
end