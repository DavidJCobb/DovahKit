
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

local define_class = nil
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
   
   local PAIRS_ITERATORS_METATABLE_KEY = "-cobb-class-helpers:pairs-iterator"
   
   --
   -- Classes need a special iterator to enable pairs() support on instances.
   --
   local _define_pairs_metatable = nil
   do -- Pairs iterator metatable
      
      --
      -- An iterator's "getters" bool indicates whether it's currently 
      -- iterating over a class-metatable's getter list. Iterators go 
      -- over the metatable's fields (i.e. methods) first, and then the 
      -- getters, before moving up to the subclass.
      --
   
      function __call(self, t, k)
         while true do
            local meta = self.meta
            if self.getters and meta then
               meta = meta.__getters
            end
            --
            if meta then
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
            end
            --
            if not self.getters then
               --
               -- Move on to the getters.
               --
               self.getters = true
            else
               --
               -- Move on to the next superclass
               --
               self.getters = false
               meta = self.meta.__superclass
               self.meta = meta
               if not meta then
                  return
               end
               k = nil -- iterating new table, so must iterate from the start
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
         local meta = getmetatable(t)
         if not meta then
            return nil
         end
         if _should_skip_name(k) then
            return nil
         end
         while true do
            local a = rawget(meta, k)
            if a then
               return a
            end
            --
            -- Check for getters:
            --
            a = rawget(meta, "_getters")
            if a then
               a = rawget(a, k)
               if a then
                  return a(t) -- remember: selfcall is just self as arg 1
               end
            end
            --
            -- Traverse up to the next superclass:
            --
            meta = meta.__superclass
            if not meta then
               return
            end
         end
      end
      
      __newindex = function(t, k, v)
         local meta = getmetatable(t)
         if not meta then
            return nil
         end
         local has_getter = false -- for error reporting
         while true do
            local a = meta.__setters
            if a then
               a = a[k]
               if a then
                  a(t, v)
                  return
               end
            end
            if not has_getter then
               a = meta.__getters
               if a and a[k] then
                  has_getter = true
               end
            end
            --
            -- Traverse up to the next superclass:
            --
            meta = meta.__superclass
            if not meta then
               break;
            end
         end
         local classname = getmetatable(t).__name or "?"
         if has_getter then
            error(string.format("DovahKIt does not allow you to assign to property '%s' on class %s", k, classname))
         end
         error(string.format("class %s does not offer a property named '%s'", classname, k))
      end
      
      __pairs = function(t)
         local iter = {
            meta    = getmetatable(t),
            getters = false,
         }
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
      super_metatable_key,
      methods,
      getters,
      setters,
      class_name
   )
      _define_pairs_metatable() -- lazy-create the pairs iterator class
      if super_metatable_key then
         assert(class_metatable_key ~= super_metatable_key, "The superclass and subclass can't use the same registry key name.")
      end
      local meta = {}
      if class_name then
         meta.__name = class_name
      end
      REGISTRY[class_metatable_key] = meta
      --
      meta.__index    = __index
      meta.__newindex = __newindex
      meta.__pairs    = __pairs
      meta.__len      = _push_zero -- #{} == 0, but #userdata == error by default
      --
      if super_metatable_key then
         local scm = REGISTRY[super_metatable_key]
         meta.__superclass = scm
         --
         -- "Operator" metamethods need to be inherited manually:
         --
         _forward_metamethod_to_subclass("__tostring", scm, meta)
         _forward_metamethod_to_subclass("__close",    scm, meta)
         _forward_metamethod_to_subclass("__gc",       scm, meta)
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
end

function cast_to_class(t, class_metatable_key)
   if not t then
      return nil
   end
   local instance_meta = getmetatable(t)
   local desired_meta  = REGISTRY[class_metatable_key]
   while desired_meta ~= instance_meta do
      local a = instance_meta.__superclass
      if type(a) ~= "table" then
         return nil
      end
      instance_meta = a
   end
   return t
end