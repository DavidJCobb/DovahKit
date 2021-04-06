local function starts_with(str, substr)
   return str:sub(1, #substr) == substr
end

local quest   = dovah.get_form_by_id(0x1EA57)
local papyrus = quest.papyrus
for k, script in pairs(papyrus.scripts) do
   dovah.log_message("Script: " .. script.name)
   for name, prop in pairs(script.properties) do
      dovah.log_message("Property: " .. name)
      if starts_with(name, "Alias_") then
         local val = prop.value
         if val then
            dovah.log_message(" - Alias ID: " .. val.id)
         else
            dovah.log_message(" - Alias property unreadable")
         end
      end
   end
end

for k, alias in pairs(quest.aliases) do
   if alias.id == 0 then
      dovah.log_message("PAPYRUS DATA ON ALIAS:")
      local papyrus = alias.papyrus
      for k, script in pairs(papyrus.scripts) do
         dovah.log_message(" - Script: " .. script.name)
      end
   end
end