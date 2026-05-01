
local topic = dovah.get_form_by_id(0x40760) -- DialogueRiftenHellos

function print_info(info)
   local resp = info.responses[1]
   if resp then
      print(string.format("[INFO:%08X] %q", info.form_id, info.responses[1].text))
   else
      print(string.format("[INFO:%08X] <no responses>", info.form_id))
   end
end

do
   local bench = benchmark.new()
   do
      local list = topic:get_infos_as_table()
      for i = 1, #list do
         print_info(list[i])
      end
   end
   bench:stop()
   print(string.format("\n\nTime via table: %s\n\n", bench:time_to_string()))
end

do
   local bench = benchmark.new()
   do
      local list = topic.infos
      for i = 1, #list do
         print_info(list[i])
      end
   end
   bench:stop()
   print(string.format("\n\nTime via native list: %s\n\n", bench:time_to_string()))
end
