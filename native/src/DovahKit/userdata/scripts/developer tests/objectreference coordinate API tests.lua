cell = dovah.get_form_by_id(0x1000D62)
assert(cell ~= nil)
assert(cell.editor_id == "DovahKitRotateTest")

do
   local w = ui.window.new()
   w.title = "opening this to allow debugging"
   w:show()
end

Group = {}
do
   local CACHED_MASK_OFFSET = false
   
   local round = math.round -- Dovahscript-provided extension; uses C++ std::round internally
   if not round then
      -- Polyfill: FPU hack from Stack Overflow
      round = function(n)
         return n + (2^52 + 2^51) + (2^52 + 2^51)
      end
   end

   Group.__index = Group
   function Group:new(statue, mask)
      assert(statue or mask)
      local instance = setmetatable({}, self)
      instance.statue   = statue
      instance.mask     = mask
      instance.position = nil
      instance.control  = false
      if statue then
         instance.position = statue.position:copy()
      else
         local p = mask.position:copy()
         p.x = round(p.x / 128) * 128
         p.y = round(p.y / 128) * 128
         p.z = 0
         instance.position = p
      end
      return instance
   end
   function Group:distance_sq(pos)
      return (self.position - pos):length_squared()
   end
   function Group:is_whole()
      if self.statue then
         return self.mask ~= nil
      end
      return false
   end
   function Group:receive(statue, mask)
      assert(statue or mask)
      local pos = nil
      if statue then
         pos = statue.position
      else
         pos = mask.position:copy()
         pos.x = round(pos.x / 128) * 128
         pos.y = round(pos.y / 128) * 128
         pos.z = 0
      end
      local a = self.position - pos
      if a:length_squared() < 64 then
         self.statue = self.statue or statue
         self.mask   = self.mask   or mask
         return true
      end
      return false
   end
   function Group:rotate(rot) -- euler
      local child = self.mask
      local basis = self.statue
      if not CACHED_MASK_OFFSET then
         CACHED_MASK_OFFSET = {}
         --
         local br = basis.rotation
         do -- extract relative rotation
            local parent = br:to_quaternion()
            local offset = child.rotation:to_quaternion()
            CACHED_MASK_OFFSET.rotation = (parent:inverse() * offset):to_euler()
         end
         do -- extract relative position
            local p_world = false
            local p_local = false
            local r_world = br:to_matrix()
            r_world:transpose_in_place()
            p_world = r_world * child.position
            p_local = r_world * basis.position
            --
            p_world:sub(p_local)
            CACHED_MASK_OFFSET.position = p_world
         end
      end
      --
      self.statue.rotation = rot
      --
      do -- construct position
         local position = self.statue.position
         local distance = rot:to_matrix() * CACHED_MASK_OFFSET.position
         self.mask.position = position + distance
      end
      do -- construct rotation
         local q_parent = rot:to_quaternion()
         local q_offset = CACHED_MASK_OFFSET.rotation:to_quaternion()
         self.mask.rotation = q_parent * q_offset
      end
   end
end

groups = {}

dovah.log_message("Finding groups...")
do
   local XMarker       = dovah.get_form_by_id(0x3B)
   local Krosis        = dovah.get_form_by_id(0x00061CB9)
   local StatueDibella = dovah.get_form_by_id(0x0008F965)
   --
   local refs = cell:get_all_refs()
   --
   local marker = nil
   --
   for i = 1, #refs do
      local ref    = refs[i]
      local base   = ref.base_form
      local statue = nil
      local mask   = nil
      if base == XMarker then
         marker = ref
         goto continue
      elseif base == Krosis then
         mask = ref
      elseif base == StatueDibella then
         statue = ref
      else
         goto continue -- could be architecture, etc.
      end
      --
      local found = false
      for j = 1, #groups do
         if groups[j] then
            local r = groups[j]:receive(statue, mask)
            if r then
               found = true
               break
            end
         end
      end
      if not found then
         local g = Group:new(statue, mask)
         groups[#groups + 1] = g
      end
      ::continue::
   end
   dovah.log_message("Found all groups.")
   --
   if marker then
      dovah.log_message("Excluding control group...")
      --
      local x  = nil
      local mp = marker.position:copy()
      --
      local MAX <const> = 64 * 64
      for i = 1, #groups do
         local group = groups[i]
         if group:is_whole() and group:distance_sq(mp) < MAX then
            dovah.log_message("Found control group.")
            group.control = true
            break
         end
      end
      dovah.log_message("Done searching for control group.")
   end
end
if #groups == 0 then
   error("No groups found!")
end

dovah.log_message("Rotating groups...")
do -- seed RNG
   local a = math.floor(os.clock() * 1000)
   local b = math.floor(os.clock() * 1000) >> 2
   math.randomseed(a, b)
end
for i = 1, #groups do
   local group = groups[i]
   if group:is_whole() and not group.control then
      local angle = euler.from_degrees(
         math.random(0, 359),
         math.random(0, 359),
         math.random(0, 359)
      )
      dovah.log_message("%s", angle)
      group:rotate(angle)
   end
end
dovah.log_message("Rotated %s groups.", #groups)