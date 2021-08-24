subject = ui.checkbox.new("Initial text")

local util = {}
do
   util.random_of_same_type = function(v)
      local t = type(v)
      if t == "boolean" then
         return math.random(0, 1) > 0.5
      end
      if t == "number" then
         if math.type(v) == "integer" then
            return math.tointeger(math.random(-30, 30))
         end
         return math.random() * 60 - 30
      end
      if t == "string" then
         return "String"
      end
   end
   util.revolve = function(list, value)
      if not value then
         return list[1]
      end
      local i
      for k, v in ipairs(list) do
         if v == value then
            i = k
            break
         end
      end
      if i then
         i = i + 1
         if i > #list then
            i = 1
         end
      else
         i = 1
      end
      if not i then
         i = 1
      end
      return list[i]
   end
end

test_field_setters = {}
test_field_names   = {}
do
   local STATES = { "unchecked", "indeterminate", "checked" }
   
   test_field_setters = {
      checked = function(v)
         subject.checked = v or not subject.checked
      end,
      state = function(v)
         subject.state = v or util.revolve(STATES, subject.state)
      end,
      text = function(v)
         subject.text = v or "Checkbox text"
      end,
   }
   
   local i = 1
   for k, _ in pairs(test_field_setters) do
      test_field_names[i] = k
      i = i + 1
   end
end
do -- events
   subject:on("OnChanged", "", function(state)
      dovah.log_message("event OnChecked(%s)", state)
   end)
   subject:on("OnToggled", "", function(checked)
      dovah.log_message("event OnToggled(%s)", checked)
   end)
end

function run_test_for_field(field)
   local setter = test_field_setters[field]
   assert(field)
   local prior = subject[field]
   if setter then
      setter()
   else
      local value = util.random_of_type(prior)
      if value ~= nil then
         subject[field] = value
      end
   end
   local after = subject[field]
   dovah.log_message("Testing field %s\n - Value prior: %s\n - Value after: %s", field, prior, after)
end

function run_test(t)
   local field = test_field_names[t]
   run_test_for_field(field)
end

local test_index = 1
function run_next_test()
   run_test(test_index)
   test_index = test_index + 1
   if test_index > #test_field_names then
      test_index = 1
   end
end

do
   local window = ui.window.new()
   window:set_layout("grid")
   window:add_child(subject)
   window:show()
   
   dovah.log_message("Use eval scripts to run the next test. Commands:\n - run_next_test\n - run_test_for_field")
end