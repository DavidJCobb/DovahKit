local SIGNATURES = {}
do
   local _len = { has_length = true }
   SIGNATURES = {
      [0xD8] = {},   -- Start of Image
      [0xD9] = {},   -- End of Image
      [0xC0] = _len, -- Start of Frame (baseline DCT)
      [0xC2] = _len, -- Start of Frame (progressive DCT)
      [0xC4] = _len, -- Define Huffman Table(s)
      [0xDB] = _len, -- Define Quantization Table(s)
      [0xFE] = _len, -- Comment
      [0xDA] = { has_length = true, has_entropy = true }, -- Start of Scan
      [0xDD] = { fixed_length = 4 }, -- Define Restart Interval
   }
   for i = 0, 7 do
      SIGNATURES[0xD0 + i] = {} -- Restart Marker
   end
   for i = 0, 9 do
      SIGNATURES[0xE0 + i] = _len -- Application-Specific Data
   end
end

function jpeg_skim(view)
   if dovah.type(view) ~= "binary_view" then
      error("binary_view object expected")
   end
   local size       = view.size
   local in_entropy = false
   local i = 0
   while i < size do
      local byte = view:get_uint8(i)
      if byte ~= 0xFF then
         if not in_entropy then
            return false, i, "unexpected byte", byte
         end
         i = i + 1
         goto continue
      end
      if i + 1 >= size then
         return false, i, "untyped signature or entropy 0xFF byte with no null suffix"
      end
      local next = view:get_uint8(i + 1)
      if in_entropy then
         if next == 0 or next == 0xFF then
            i = i + 1
            goto continue
         end
         in_entropy = false
      end
      local info = SIGNATURES[next]
      if not info then
         return false, i + 1, "unknown signature", next
      end
      local skip_count = 2 -- size of header
      if info.has_length then
         if i + 3 >= size then
            return false, i + 2, "EOF before length"
         end
         local length = view:get_uint16({ offset = i + 2, endian = "big" })
         length = length - 2
         if length < 0 then
            return false, i + 2, "bad length", length
         end
         --
         skip_count = skip_count + 2 -- skip the size of the length itself
         skip_count = skip_count + length
      elseif info.fixed_length then
         skip_count = skip_count + info.fixed_length
      end
      i = i + skip_count
      --
      if info.has_entropy then
         in_entropy = true
      end
      ::continue::
   end
   return true
end