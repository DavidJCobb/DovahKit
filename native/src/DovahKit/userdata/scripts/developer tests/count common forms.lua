local total = 0
local types = {}
for k,v in pairs(form_types) do
   local count = 0
   dovah.for_each_form_of_type(v, function(form)
      count = count + 1
      total = total + 1
   end)
   --
   local signature = string.char(
      (v >> 0x18) & 0xFF,
      (v >> 0x10) & 0xFF,
      (v >> 0x08) & 0xFF,
      v & 0xFF
   )
   types[signature] = count
end

types["CELL"] = 590 -- interior cell count in Skyrim.esm; done manually until we have a cell wrapper and a func to check whether one is an exterior

local refr = { "ACHR", "REFR" }
local unconventional = { "INFO", "LAND", "NAVI", "NAVM" }

local common = total
for _, v in ipairs(unconventional) do
   common = common - (types[v] or 0)
end
for _, v in ipairs(refr) do
   common = common - (types[v] or 0)
end

function table.contains(t, v)
   for a, b in pairs(t) do
      if b == v then
         return true
      end
   end
   return false
end

dovah.log_message("Total: %d (%d common)", total, common)
for k,v in pairs(types) do
   if not table.contains(refr, k) and not table.contains(unconventional, k) then
      if (v / common) > 0.01 then
         dovah.log_message("%s: %d (%f%% of non-ref)", k, v, (v / common * 100))
      end
   end
end