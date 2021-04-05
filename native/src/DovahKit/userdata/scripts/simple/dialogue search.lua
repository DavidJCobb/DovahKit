local window   = ui.window.new()
local quest    = ui.formpicker.new()
local speaker  = ui.formpicker.new()
local substr   = ui.textbox.new()
local button   = ui.button.new("Search")
local progress = ui.progress_bar.new()

quest.form_types = form_types.quest
speaker.form_types = { form_types.actor_base, form_types.voicetype }

progress.format = "%p% ( %v / %m )"
progress.alignment = "center center"

window:set_layout("grid")
window:add_child(ui.text.new("Quest:"),     1, 1)
window:add_child(ui.text.new("Speaker:"),   2, 1)
window:add_child(ui.text.new("Substring:"), 3, 1)
window:add_child(quest,    1, 2)
window:add_child(speaker,  2, 2)
window:add_child(substr,   3, 2)
window:add_child(progress, 4, 1, 1, 2)
do
   local strip  = ui.widget.new()
   strip:set_layout("ltr")
   strip:add_child(button)
   strip.layout_margins = 0
   window:add_child(strip, 5, 1, 1, 2)
end

local search_parameters = {
   speaker   = nil, -- NOT IMPLEMENTED
   quest     = nil, -- NOT IMPLEMENTED
   substring = "",
}

function set_widget_states(enabled)
   button.enabled  = enabled
   quest.enabled   = enabled
   speaker.enabled = enabled
   substr.enabled  = enabled
end

function on_match_found(info)
   --
   -- TODO: have a table view and display results in there
   --
   local text = "[INFO:" .. info:form_id_to_string() .. "]"
   for _, response in ipairs(info.responses) do
      text = text .. " " .. response.text
   end
   dovah.log_message(text)
end

function search()
   progress.maximum = dovah.count_forms_of_type(form_types.topic_info)
   progress.value   = 0
   --
   local count = 0
   dovah.for_each_form_of_type(form_types.topic_info, function(info)
      count = count + 1
      progress.value = count
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

window:show()