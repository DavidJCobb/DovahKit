local window   = ui.window.new()
local quest    = ui.formpicker.new()
local speaker  = ui.formpicker.new()
local substr   = ui.textbox.new()
local button   = ui.button.new("Search")
local progress = ui.progress_bar.new()
local table    = ui.table_view.new()

quest.form_types = form_types.quest
speaker.form_types = { form_types.actor_base, form_types.voicetype }
speaker.enabled = false -- TODO: IMPLEMENT ME

progress.format = "%p% ( %v / %m )"
progress.alignment = "center center"

table.column_headers = { "Quest", "Form ID", "Editor ID", "Info Text" }
table.selection_mode = "single"
table.show_row_headers = false
table.word_wrap = "truncate"

window:set_layout("grid")
do
   local search_form = ui.widget.new()
   search_form:set_layout("grid")
   search_form:add_child(ui.text.new("Quest:"),     1, 1)
   search_form:add_child(ui.text.new("Speaker:"),   2, 1)
   search_form:add_child(ui.text.new("Substring:"), 3, 1)
   search_form:add_child(quest,    1, 2)
   search_form:add_child(speaker,  2, 2)
   search_form:add_child(substr,   3, 2)
   search_form:add_child(button, 1, 3, 3, 1)
   search_form.layout_margins = 0
   window:add_child(search_form, 1, 1)
end
window:add_child(progress, 2, 1)
window:add_child(table, 3, 1)

local search_parameters = {
   speaker   = nil, -- NOT IMPLEMENTED
   quest     = nil,
   substring = "",
}

function set_widget_states(enabled)
   button.enabled  = enabled
   quest.enabled   = enabled
   speaker.enabled = enabled
   substr.enabled  = enabled
   if enabled then
      table.selection_mode = "single"
   else
      table.selection_mode = "disabled"
   end
end

function on_match_found(info)
   local text = nil
   for _, response in ipairs(info.responses) do
      if text then
         text = text .. " " .. response.text
      else
         text = response.text
      end
   end
   if not text then
      text = ""
   end
   local quest = info.parent_quest
   table:append_row(quest.editor_id, info:form_id_to_string(), info.editor_id, text)
end

function search()
   local form_count = dovah.count_forms_of_type(form_types.topic_info)
   progress.maximum = form_count
   progress.value   = 0
   if form_count == 0 then
      dovah.log_message("There are no infos to search.")
      progress.maximum = 1
      progress.value   = 1
      set_widget_states(true)
      return
   end
   --
   if object_is_zombie(search_parameters.quest) then
      search_parameters.quest = nil
   end
   --
   local count = 0
   dovah.for_each_form_of_type(form_types.topic_info, function(info)
      count = count + 1
      progress.value = count
      if search_parameters.quest and info.parent_quest ~= search_parameters.quest then
         return
      end
      for i, response in ipairs(info.responses) do
         if string.find(response.text, search_parameters.substring, 1, true) then
            on_match_found(info)
            return
         end
      end
   end)
   set_widget_states(true)
end

button:on("OnActivated", "", function()
   set_widget_states(false)
   search_parameters.speaker   = speaker.form
   search_parameters.quest     = quest.form
   search_parameters.substring = substr.text
   dovah.log_message("Searching...")
   ui.run_when_unlocked(search)
end)

local last_selected_info = nil
local selected_response  = 1

local show_num  = ui.text.new("")
local show_text = ui.text.new("")
function update_inspect()
   show_num.text = ""
   show_text.text = ""
   if not last_selected_info then
      return
   end
   local list  = last_selected_info.responses
   local count = #list
   if selected_response < 1 then
      selected_response = 1
   end
   if selected_response > count then
      selected_response = count
   end
   show_num.text = string.format("%.0f", selected_response)
   local response = list[selected_response]
   if not response then
      return
   end
   show_text.text = response.text
end

do
   local btn_prev = ui.button.new("<")
   local btn_next = ui.button.new(">")
   local r_strip  = ui.widget.new()
   do
      r_strip:set_layout("ltr")
      r_strip:add_child(btn_prev)
      r_strip:add_child(show_num)
      r_strip:add_child(btn_next)
   end
   
   show_text.word_wrap = true
   
   local inspect = ui.widget.new()
   inspect:set_layout("grid")
   inspect:add_child(ui.text.new("Response #"), 1, 1)
   inspect:add_child(r_strip, 1, 2)
   inspect:add_child(ui.text.new("Text"), 2, 1)
   inspect:add_child(show_text, 2, 2)
   inspect.layout_margins = 0
   
   btn_prev:on("OnActivated", "", function()
      selected_response = selected_response - 1
      update_inspect()
   end)
   btn_next:on("OnActivated", "", function()
      selected_response = selected_response + 1
      update_inspect()
   end)
   
   window:add_child(inspect, 4, 1)
end

table:on("OnSelectionChanged", "", function(row, ...)
   if not row then
      last_selected_info = nil
      update_inspect()
      return
   end
   local cell = row.cells[2]
   if not cell then
      error("row has no formID cell?!")
   end
   local text = cell.text
   local id   = tonumber(text, 16)
   local form = dovah.get_form_by_id(id)
   last_selected_info = form
   update_inspect()
end)

window:show()