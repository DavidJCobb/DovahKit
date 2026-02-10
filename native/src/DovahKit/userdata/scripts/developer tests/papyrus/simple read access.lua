
-- WeaponRackCOARight
do
   local form    = dovah.get_form_by_id(0x000E49BC)
   local papyrus = form.papyrus
   do
      local script = papyrus.scripts["WeaponRackTriggerSCRIPT"]
      if not script then
         error("script missing")
      end
      if script ~= papyrus.scripts["weaponracktriggerSCRIPT"] then
         error("script collection is not case-insensitive")
      end
      if script ~= papyrus.scripts[1] then
         error("something wrong with script collection name/index mapping")
      end
      print("[ACTI:000E49BC]WeaponRackCOARight")
      print(" - WeaponRackTriggerSCRIPT")
      print("    = Status: " .. script.status)
      print("    - Properties:")
      local props = script.properties
      local count = #props
      for i = 1, count do
         local prop = props[i]
         print("       - Name: " .. prop.name)
         print("       - Status: " .. prop.status)
         local v = prop.value
         dovah.dump(v) -- TODO: print in more detail
      end
   end
end

-- Ref in RiftenRaggedFlagon
do
   local form    = dovah.get_form_by_id(0x000DB7AB)
   local papyrus = form.papyrus
   do
      local script = papyrus.scripts["WeaponRackActivateSCRIPT"]
      if not script then
         error("script missing")
      end
      if script ~= papyrus.scripts["weaponRaCkactivateScRiPt"] then
         error("script collection is not case-insensitive")
      end
      if script ~= papyrus.scripts[1] then
         error("something wrong with script collection name/index mapping")
      end
      print("[REFR:000DB7AB]WeaponRackCOARight")
      print(" - WeaponRackActivateSCRIPT")
      print("    = Status: " .. script.status)
      print("    - Properties:")
      local props = script.properties
      local count = #props
      for i = 1, count do
         local prop = props[i]
         print("       - Name: " .. prop.name)
         print("       - Status: " .. prop.status)
         local v = prop.value
         dovah.dump(v) -- TODO: print in more detail
      end
   end
end

-- Alias in ArenaWagerFighterQuest
do
   local quest   = dovah.get_form_by_id(0x000DC9DF)
   local alias   = quest.aliases["Fighter"]
   local papyrus = alias.papyrus
   local script  = papyrus.scripts["ArenaWagerFighterScript"]
   if not script then
      error("script missing")
   end
      print("[QUST:000DC9DF]ArenaWagerFighterQuest/[Alias#0]Fighter")
      print(" - ArenaWagerFighterScript")
end
