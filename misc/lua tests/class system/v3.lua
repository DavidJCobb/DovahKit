
--
-- VERSION 3 of the LUA CLASS SYSTEM
--
-- New implementation, intended to support multiple inheritance. Seems to 
-- work so far.
--


local REGISTRY = {}

local _is_metamethod_name = function(name)
   return ({
      __add = true,
      __band = true,
      __bnot = true,
      __bor = true,
      __bxor = true,
      __call = true,
      __close = true,
      __concat = true,
      __div = true,
      __eq = true,
      __gc = true,
      __idiv = true,
      __index = true,
      __le = true,
      __len = true,
      __lt = true,
      __metatable = true,
      __mod = true,
      __mode = true,
      __mul = true,
      __name = true,
      __newindex = true,
      __pairs = true,
      __pow = true,
      __shl = true,
      __shr = true,
      __sub = true,
      __tostring = true,
      __unm = true,
   })[name]
end -- library function

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
         or n == "__superclasses"
         or n == "__classlist"
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
      
   
      function __call(self, t, k)
         if getmetatable(t) ~= self.meta then
            return
         end
         if k then
            self.seen[k] = true
         end
         local v     = nil
         local list  = rawget(self.meta, "__classlist")
         local count = #list
         for i = count - self.done, 1, -1 do
            local cls = list[i]
            if not self.getters then
               k, v = next(cls, k)
               while _should_skip_name(k) or self.seen[k] do
                  k, v = next(cls, k)
               end
               if k then
                  return k, v
               end
               self.getters = true
            end
            if self.getters then
               cls = cls.__getters
               --
               k, v = next(cls, k)
               while _should_skip_name(k) or self.seen[k] do
                  k, v = next(cls, k)
               end
               if k then
                  if v then
                     v = (v)(t)
                  end
                  return k, v
               end
               --
               -- No getters or all getters iterated; move on to 
               -- next class.
               --
               self.done    = self.done + 1
               self.getters = false
            end
         end
         return nil, nil -- implicit
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
         local meta  = getmetatable(t)
         local list  = rawget(meta, "__classlist")
         local count = #list
         for i = count, 1, -1 do
            local cls = list[i]
            local a   = rawget(cls, k)
            if a then
               return a
            end
            a = rawget(cls, "__getters")
            if a then
               a = rawget(a, k)
               if a then
                  return a(t)
               end
            end
         end
      end
      
      __newindex = function(t, k, v)
         if _should_skip_name(k) then
            return nil
         end
         local has_getter = false
         local meta  = getmetatable(t)
         local list  = rawget(meta, "__classlist")
         local count = #list
         for i = count, 1, -1 do
            local cls = list[i]
            local a   = rawget(cls, "__setters")
            if a then
               a = a[k]
               if a then
                  a(t, v)
                  return
               end
            end
            if not has_getter then
               a = rawget(meta, "__getters")
               if a and a[k] then
                  has_getter = true
               end
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
            seen    = {},
            done    = 0,
            getters = false,
         }
         setmetatable(iter, REGISTRY[PAIRS_ITERATORS_METATABLE_KEY])
         return iter, t, nil
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
      meta.__classlist = {} -- list including all ancestor classes and itself
      if super_metatable_keys then
         local count = #super_metatable_keys
         if count > 0 then
            meta.__superclasses = {}
            --
            local supers = {}
            do
               local flat_count = 1
               local seen       = {}
               for i = 1, count do
                  local name  = super_metatable_keys[i]
                  local super = REGISTRY[name]
                  if super == meta then
                     error("A class cannot be its own superclass.")
                  end
                  supers[i] = super
                  --
                  local sc = #super.__classlist
                  for j = 1, sc do
                     local cls = super.__classlist[j]
                     if seen[cls] then
                        error(string.format("New class '%s' has diamond inheritance of '%s'.", meta.__name, cls.__name))
                     end
                     meta.__classlist[flat_count] = cls
                     seen[cls]  = true
                     flat_count = flat_count + 1
                  end
               end
            end
            --
            -- Iterate the list in reverse order, so that when we forward the 
            -- metamethods from superclasses to the subclass, the later super-
            -- classes in the list take precedence over the earlier ones.
            --
            for i = count, 1, -1 do
               local super = supers[i]
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
      meta.__classlist[#meta.__classlist + 1] = meta
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
      local meta = getmetatable(t)
      if not meta then
         return nil
      end
      if meta == desired then
         return t
      end
      local list = rawget(meta, "__classlist")
      for i = 1, #list do
         if desired == list[i] then
            return t
         end
      end
      return nil
   end
end