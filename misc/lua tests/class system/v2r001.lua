
--
-- VERSION 1 of the LUA CLASS SYSTEM
--
-- Ported from the C++ implementation. Intended to allow robust class functionality 
-- to be applied to userdata.
--
-- Currently supports inheritance, but not multiple inheritance or mixins.
--


local REGISTRY = {}

local _is_metamethod_name = function() end -- library function

local define_class  = nil
local cast_to_class = nil
do
   -- Return true if a field name is internal to the class system and should not be 
   -- accessible on class instances.
   function _should_skip_name(n)
      if type(n) == "string" then
         if _is_metamethod_name(n)
         or n == "__getters"
         or n == "__setters"
         or n == "__superclass"
         or n == "__name"
         then
            return true
         end
      end
      return false
   end
   
   function _for_each_class(t, functor, meta)
      if not meta then
         meta = getmetatable(t)
         if not meta then
            return
         end
      end
      --
      -- If there's only one superclass, then we should process it with a 
      -- loop, not with recursion. Let's loop as far as we can before we 
      -- encounter multiple inheritance, and then we'll recurse.
      --
      while true do
         local result, stop = functor(meta)
         if stop then
            return result, stop
         end
         local list = meta.__superclasses
         if not list then
            return
         else
            local count = #list
            if count == 1 then
               meta = list[1]
            elseif count > 1 then
               for i = count, 1, -1 do
                  assert(list[i])
                  result, stop = _for_each_class(t, functor, list[i])
                  if stop then
                     return result, stop
                  end
               end
               break
            else
               break
            end
         end
      end
   end
   
   local PAIRS_ITERATORS_METATABLE_KEY = "-cobb-class-helpers:pairs-iterator"
   
   --
   -- Classes need a special iterator to enable pairs() support on instances.
   --
   local _define_pairs_metatable = nil
   do -- Pairs iterator metatable
      
      --
      -- Whatever solution we take for pairs() must account for the 
      -- possibility that the target object's metatable can change 
      -- during a pairs() loop, e.g. if you use pairs() on a form 
      -- and potentially delete the form in the loop body, which 
      -- would zombify the userdata (change its metatable).
      --
      
      --
      -- An iterator's "getters" bool indicates whether it's currently 
      -- iterating over a class-metatable's getter list. Iterators go 
      -- over the metatable's fields (i.e. methods) first, and then the 
      -- getters, before moving up to the subclass.
      --
      
      --
      -- TODO: Iterators now need to store more state, to cope with any 
      --       multiple inheritance.
      --
   
      function __call(self, t, k)
         error("TODO: UNTESTED")
         error("TODO: If fields are overridden or exist on multiple classes in the inheritance tree, this will probably iterate them multiple times")
         for i = self.place, #self.pending do
            self.place = i
            if i > 1 then
               self.pending[i - 1] = nil
            end
            local meta = self.pending[i]
            if meta then
               --
               -- We need to m
               --
               if not self.getters then
                  local k, v = next(meta)
                  while _should_skip_name(k) do
                     k, v = next(meta)
                  end
                  if v ~= nil then
                     if self.getters then
                        v = (v)(t)
                     end
                     return k, v
                  end
                  --
                  self.getters = true
               end
               if self.getters then
                  meta = meta.__getters
                  --
                  local k, v = next(meta)
                  while _should_skip_name(k) do
                     k, v = next(meta)
                  end
                  if v ~= nil then
                     v = (v)(t)
                     return k, v
                  end
                  --
                  self.getters = false
               end
            end
         end
      end
      
      _define_pairs_metatable = function()
         local defined = type(REGISTRY[PAIRS_ITERATORS_METATABLE_KEY]) == "table"
         if defined then
            return
         end
         local meta = {}
         REGISTRY[PAIRS_ITERATORS_METATABLE_KEY] = meta
         meta.__call = __call
      end
   end

   local __index    = nil
   local __newindex = nil
   local __pairs    = nil
   do -- Metamethods
      __index = function(t, k)
         if _should_skip_name(k) then
            return nil
         end
         local result = _for_each_class(t, function(meta)
--print("__index checking metatable for field " .. k .. ": " .. meta.__name)
            local a = rawget(meta, k)
            if a then
--print("__index got field " .. k .. " from " .. meta.__name)
               return a, true
            end
            --
            -- Check for getters:
            --
            a = rawget(meta, "__getters")
            if a then
               a = rawget(a, k)
               if a then
                  return a(t), true
               end
            end
         end)
         return result
      end
      
      __newindex = function(t, k, v)
         if _should_skip_name(k) then
            return nil
         end
         local has_getter = false
         local matched    = _for_each_class(t, function(meta)
--print("__newindex checking metatable for field " .. k .. ": " .. meta.__name)
            local a = rawget(meta, "__setters")
            if a then
               a = a[k]
               if a then
--print("__newindex got field " .. k .. " from " .. meta.__name)
                  a(t, v)
                  return true, true
               end
            end
            if not has_getter then
               a = rawget(meta, "__getters")
               if a and a[k] then
                  has_getter = true
               end
            end
         end)
         if not matched then
            local classname = getmetatable(t).__name or "?"
            if has_getter then
               error(string.format("DovahKIt does not allow you to assign to property '%s' on class %s", k, classname))
            end
            error(string.format("class %s does not offer a property named '%s'", classname, k))
         end
      end
      
      __pairs = function(t)
         local iter = {
            pending = {},
            place   = 1,
            getters = false,
         }
         _for_each_class(t, function(meta)
            iter.pending[#iter.pending] = meta
         end)
         setmetatable(iter, REGISTRY[PAIRS_ITERATORS_METATABLE_KEY])
         return iter
      end
   end
   
   function _push_zero(self)
      return 0
   end
   function _forward_metamethod_to_subclass(metamethod, super, sub)
      if sub[metamethod] ~= nil then
         return
      end
      if super[metamethod] ~= nil then
         sub[metamethod] = super[metamethod]
      end
   end

   define_class = function(
      class_metatable_key,
      super_metatable_keys,
      methods,
      getters,
      setters,
      class_name
   )
      _define_pairs_metatable() -- lazy-create the pairs iterator class
      if super_metatable_keys then
         for _, v in ipairs(super_metatable_keys) do
            assert(class_metatable_key ~= v, "A superclass and subclass can't use the same registry key name.")
         end
      end
      local meta = {}
      if class_name then
         meta.__name = class_name
      else
         meta.__name = class_metatable_key
      end
      REGISTRY[class_metatable_key] = meta
      --
      meta.__index    = __index
      meta.__newindex = __newindex
      meta.__pairs    = __pairs
      meta.__len      = _push_zero -- #{} == 0, but #userdata == error by default
      --
      if super_metatable_keys then
         local count = #super_metatable_keys
         if count > 0 then
            meta.__superclasses = {}
            --
            -- Iterate the list in reverse order, so that when we forward the 
            -- metamethods from superclasses to the subclass, the later super-
            -- classes in the list take precedence over the earlier ones.
            --
            for i = count, 1, -1 do
               local name  = super_metatable_keys[i]
               local super = REGISTRY[name]
               meta.__superclasses[i] = super
               --
               -- "Operator" metamethods need to be inherited manually:
               --
               _forward_metamethod_to_subclass("__tostring", super, meta)
               _forward_metamethod_to_subclass("__close",    super, meta)
               _forward_metamethod_to_subclass("__gc",       super, meta)
            end
         end
      end
      --
      if methods then
         for k, v in pairs(methods) do
            meta[k] = v
         end
      end
      if getters then
         meta.__getters = {}
         for k, v in pairs(getters) do
            meta.__getters[k] = v
         end
      end
      if setters then
         meta.__setters = {}
         for k, v in pairs(setters) do
            meta.__setters[k] = v
         end
      end
   end
   
   cast_to_class = function(t, class_metatable_key)
      local desired = REGISTRY[class_metatable_key]
      if not desired then
         return nil
      end
      local matched = _for_each_class(t, function(meta)
         if meta == desired then
            return true, true
         end
      end)
      if matched then
         return t
      end
      return nil
   end
end