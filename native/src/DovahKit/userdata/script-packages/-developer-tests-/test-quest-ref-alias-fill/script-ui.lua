
script_ui = {
   window = ui.window.new(),
   quest  = ui.formpicker.new(),
   alias  = ui.dropdown.new(),
   fill = {
      at_location = {
         radio        = ui.radio_button.new("Find at location: "),
         alias        = ui.dropdown.new(),
         loc_ref_type = ui.formpicker.new()
      },
      create = {
         radio         = ui.radio_button.new("Create: "),
         base_form     = ui.formpicker.new(),
         difficulty    = ui.dropdown.new(),
         at_alias      = ui.dropdown.new(),
         in_inventory  = ui.dropdown.new(),
         init_disabled = ui.checkbox.new("Initially disabled"),
      },
      event_data = {
         radio = ui.radio_button.new("Find by event data: "),
         data  = ui.textbox.new()
      },
      external_alias = {
         radio = ui.radio_button.new("Copy from external alias: "),
         quest = ui.formpicker.new(),
         alias = ui.dropdown.new(),
      },
      linked_ref = {
         radio = ui.radio_button.new("Find linked ref of: "),
         alias = ui.dropdown.new(),
      },
      loaded_closest = {
         radio = ui.radio_button.new("Find in loaded area (closest)")
      },
      loaded_random = {
         radio = ui.radio_button.new("Find in loaded area (random)")
      },
      preassigned = {
         radio = ui.radio_button.new("Preassigned ref: "),
         -- TODO
      },
      unique_actor = {
         radio = ui.radio_button.new("Unique actor: "),
         actor = ui.formpicker.new()
      }
   },
   commit = ui.button.new("Commit changes")
}

--
-- Set up editing controls
--

script_ui.quest.form_types = { form_types.quest }

script_ui.fill.at_location.loc_ref_type.form_types = { form_types.location_ref_type }

script_ui.fill.create.base_form.form_types = {
   form_types.activator,
   form_types.actor_base,
   form_types.ammo,
   form_types.armor,
   form_types.book,
   form_types.container,
   form_types.door,
   form_types.flora,
   form_types.furniture,
   form_types.ingredient,
   form_types.key,
   form_types.leveled_item,
   form_types.light,
   form_types.misc_item,
   form_types.movable_static,
   form_types.potion,
   form_types.soul_gem,
   form_types.static,
   form_types.tree,
   form_types.weapon,
}
do
   local values = {
      "easy",
      "medium",
      "hard",
      "very hard",
      "none"
   }
   local widget = script_ui.fill.create.difficulty
   for i = 1, #values do
      widget:append_item(values[i])
   end
end
do
   local widget = script_ui.fill.create.in_inventory
   widget:append_item("at")
   widget:append_item("in")
end

script_ui.fill.external_alias.quest.form_types = { form_types.quest }

script_ui.fill.unique_actor.actor.form_types = { form_types.actor_base }

--
-- Build window
--

do
   local window = script_ui.window
   window.title = "Edit quest ref alias"
   window:set_layout("grid")
   
   local row = 1
   
   do
      local label  = ui.text.new("Quest:")
      local widget = script_ui.quest
      window:add_child(label,  row, 1)
      window:add_child(widget, row, 2)
      row = row + 1
   end
   do
      local label  = ui.text.new("Alias:")
      local widget = script_ui.alias
      window:add_child(label,  row, 1)
      window:add_child(widget, row, 2)
      row = row + 1
   end
   
   local groupbox = ui.groupbox.new("Fill")
   groupbox:set_layout("grid")
   window:add_child(groupbox, row, 1, 1, 2)
   do
      local row = 1
      do
         local fill = script_ui.fill.at_location
         groupbox:add_child(fill.radio, row, 1)
         groupbox:add_child(fill.alias, row, 2)
         groupbox:add_child(ui.text.new(" with LocRefType "), row, 3)
         groupbox:add_child(fill.loc_ref_type, row, 4)
         row = row + 1
      end
      do
         local fill = script_ui.fill.event_data
         groupbox:add_child(fill.radio, row, 1)
         groupbox:add_child(fill.data,  row, 2)
         row = row + 1
      end
      do
         local fill = script_ui.fill.create
         groupbox:add_child(fill.radio,         row, 1)
         groupbox:add_child(fill.base_form,     row, 2)
         groupbox:add_child(ui.text.new(" with difficulty "), row, 3)
         groupbox:add_child(fill.difficulty,    row, 4)
         groupbox:add_child(fill.in_inventory,  row, 5)
         groupbox:add_child(fill.at_alias,      row, 6)
         groupbox:add_child(fill.init_disabled, row, 7)
         row = row + 1
      end
      do
         local fill = script_ui.fill.external_alias
         groupbox:add_child(fill.radio, row, 1)
         groupbox:add_child(fill.quest, row, 2)
         groupbox:add_child(fill.alias, row, 3)
         row = row + 1
      end
      do
         local fill = script_ui.fill.loaded_closest
         groupbox:add_child(fill.radio, row, 1)
         row = row + 1
      end
      do
         local fill = script_ui.fill.loaded_random
         groupbox:add_child(fill.radio, row, 1)
         row = row + 1
      end
      do
         local fill = script_ui.fill.linked_ref
         groupbox:add_child(fill.radio, row, 1)
         groupbox:add_child(fill.alias, row, 2)
         row = row + 1
      end
      do
         local fill = script_ui.fill.preassigned
         groupbox:add_child(fill.radio, row, 1)
         row = row + 1
      end
      do
         local fill = script_ui.fill.unique_actor
         groupbox:add_child(fill.radio, row, 1)
         groupbox:add_child(fill.actor, row, 2)
         row = row + 1
      end
   end
   row = row + 1
   do
      window:add_child(script_ui.commit, row, 1, 1, 2)
      row = row + 1
   end
end

--
-- Utils
--

function fill_alias_dropdown(widget, alias_type, before, quest)
   widget:clear()

   if not quest then
      quest = script_ui.quest.form
      if not quest then
         return
      end
   end
   local list = quest.aliases
   local size = #list
   if size == 0 then
      widget.enabled = false
      return
   else
      widget.enabled = true
   end
   for i = 1, size do
      local alias    = list[i]
      if alias.type == alias_type then
         local alias_id = alias.id
         local keep     = true
         if before and alias_id >= before.id then
            keep = false
         end
         if keep then
            widget:append_item(string.format("%d: %s", alias_id, alias.name))
         end
      end
   end
end
function select_alias_in_dropdown(widget, alias)
   local label = string.format("%d: %s", alias.id, alias.name)
   widget.selected_text = label
end
function get_selected_alias(widget, quest)
   if not quest then
      quest = script_ui.quest.form
      if not quest then
         return
      end
   end
   local text     = widget.selected_text or ""
   local id, name = text:match("^(%d+): (.*)$")
   if not id then
      return
   end
   id = tonumber(id)
   if not id then
      return
   end
   
   return quest.aliases_by_id[id]
end

function get_alias_of_interest()
   return get_selected_alias(script_ui.alias, script_ui.quest.form)
end

function fill_params_to_ui(alias)
   fill_alias_dropdown(script_ui.fill.at_location.alias, "location",  alias)
   fill_alias_dropdown(script_ui.fill.create.at_alias,   "reference", alias)
   fill_alias_dropdown(script_ui.fill.linked_ref.alias,  "reference", alias)

   local fill = alias.fill
if fill and fill.type then dovah.dump(fill.type) end
dovah.dump(fill)
   if object_is_form(fill) then
      if fill.form_type == form_types.actor_base then
         script_ui.fill.unique_actor.radio.checked = true
         script_ui.fill.unique_actor.actor.form    = fill
         return
      end
      if fill.form_type == form_types.reference then
         script_ui.fill.preassigned.radio.checked = true
         -- TODO
         return
      end
   end
   if fill.type == "at location" then
      local p = script_ui.fill.at_location
      p.radio.checked = true
      select_alias_in_dropdown(p.alias, fill.alias)
      p.loc_ref_type.form = fill.loc_ref_type
      return
   end
   if fill.type == "create" then
      local p = script_ui.fill.create
      p.radio.checked = true
      p.base_form.form = fill.base_form
      p.difficulty.selected_text = fill.difficulty
      select_alias_in_dropdown(p.at_alias, fill.create_at)
      if fill.create_in_inventory then
         p.in_inventory.selected_index = 2
      else
         p.in_inventory.selected_index = 1
      end
      p.init_disabled.checked = fill.initially_disabled
      return
   end
   if fill.type == "linked ref of" then
      local p = script_ui.fill.linked_ref
      p.radio.checked = true
      select_alias_in_dropdown(p.alias, fill.alias)
      return
   end
   
   if type(fill) == "string" then
      if fill == "find closest in loaded area" then
         script_ui.fill.loaded_closest.radio.checked = true
         return
      end
      if fill == "find random in loaded area" then
         script_ui.fill.loaded_random.radio.checked = true
         return
      end
      script_ui.fill.event_data.radio.checked = true
      script_ui.fill.event_data.data.text = fill
      return
   end
   
   -- assume fill is an external alias
   if fill then
      local p = script_ui.fill.external_alias
      p.radio.checked = true
      local quest = fill.parent
      if quest then
         p.quest.form = quest
         fill_alias_dropdown(p.alias, "reference", nil, quest)
         select_alias_in_dropdown(p.alias, fill)
      end
   end
end

--
-- Behavior
--

script_ui.alias:on("OnChanged", "", function()
   local alias = get_alias_of_interest()
   if not alias then
      dovah.log_message("no alias selected?")
      return
   end
   fill_params_to_ui(alias)
end)
script_ui.quest:on("OnChanged", "", function(stub)
   dovah.log_message("quest changed")
   fill_alias_dropdown(script_ui.alias, "reference")
end)

script_ui.commit:on("OnActivated", "", function()
   local alias = get_alias_of_interest()
   if not alias then
      return
   end
   
   do -- at location
      local p = script_ui.fill.at_location
      if p.radio.checked then
         alias.fill = {
            type         = "at location",
            alias        = get_selected_alias(p.alias),
            loc_ref_type = p.loc_ref_type.form
         }
         return
      end
   end
   do -- create
      local p = script_ui.fill.create
      if p.radio.checked then
         alias.fill = {
            type                = "create",
            base_form           = p.base_form.form,
            create_at           = get_selected_alias(p.at_alias),
            create_in_inventory = p.in_inventory.selected_text == "in",
            difficulty          = p.difficulty.selected_text,
            initially_disabled  = p.init_disabled.checked,
         }
         return
      end
   end
   do -- event data
      local p = script_ui.fill.event_data
      if p.radio.checked then
         alias.fill = p.data.text
         return
      end
   end
   do -- external alias
      local p = script_ui.fill.external_alias
      if p.radio.checked then
         alias.fill = get_selected_alias(p.alias, p.quest.form)
         return
      end
   end
   do -- linked ref
      local p = script_ui.fill.linked_ref
      if p.radio.checked then
         alias.fill = {
            type  = "linked ref of",
            alias = get_selected_alias(p.alias),
         }
         return
      end
   end
   do -- loaded, closest
      local p = script_ui.fill.loaded_closest
      if p.radio.checked then
         alias.fill = "find closest in loaded area"
         return
      end
   end
   do -- loaded, random
      local p = script_ui.fill.loaded_random
      if p.radio.checked then
         alias.fill = "find random in loaded area"
         return
      end
   end
   do -- preassigned
      local p = script_ui.fill.preassigned
      if p.radio.checked then
         -- TODO
         return
      end
   end
   do -- unique actor
      local p = script_ui.fill.unique_actor
      if p.radio.checked then
         alias.fill = p.actor.form
         return
      end
   end
end)


script_ui.window:show()