local M = {}

function M.encode(value)
    local bytes = {}
    for i = 1, #value do
        bytes[i] = ("%02X"):format(value:byte(i))
    end
    return table.concat(bytes, " ")
end

return M
