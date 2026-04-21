
local test_infos = {
   0x00013113,
   0x00016F92
}

local response_keys = {
   "edits",
   "emotion_type",
   "emotion_value",
   "listener_idle",
   "script_notes",
   "speaker_idle",
   "substitute_sound",
   "text",
   "unique_id",
}


for _, id in pairs(test_infos) do
   local info = dovah.get_form_by_id(id)
   print(string.format("[%s:%08X]", info.form_type.signature, id))
   
   do
      local list = info.responses
      local size = #list
      print(string.format(" - Responses (%u)", size))
      for i = 1, size do
         local resp = list[i]
         print(string.format("    - Response #%u", i))
         for _, k in ipairs(response_keys) do
            local v = resp[k]
            if dovah.object_is(v, "form") then
               v = string.format("[%s:%s]%s", v.form_type.signature, v:form_id_to_string(), v.editor_id)
            end
            print(string.format("      - %q == %q", k, v))
         end
      end
   end
end

do
   local info
   do
      local quest = dovah.create_form(form_types.quest)
      quest.editor_id = "aaaTESTQuest"

      local branch = dovah.create_form(form_types.dialogue_branch, { parent = quest })
      branch.editor_id = "aaaTESTBranch"

      local topic = dovah.create_form(form_types.topic, { parent = branch })
      topic.editor_id = "aaaTESTTopic"
      
      info = dovah.create_form(form_types.topic_info, { parent = topic })
   end
   info.responses:insert({
      text = "Response 1"
   })
   info.responses:insert({
      emotion_type = "anger",
      text         = "Response 3"
   })
   info.responses:insert(2, {
      text = "Response 2"
   })
   
   info.responses[2]:assign({
      script_notes = "Script Notes 2"
   })
   info.responses[3].emotion_value = 65
   info.responses[3]:overwrite_with({
      emotion_value = 55,
      text          = "Response 3, overwritten"
   })
   
   
   info.responses:insert(6, {
      text      = "Response 4",
      unique_id = 69,
   })
end