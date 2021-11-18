function fib(n)
  if n < 2 then
    return n
  end
  return fib(n - 2) + fib(n - 1)
end

local n = tonumber(arg[1])
local m = tonumber(arg[2])
for i = 1, n do
  print(fib(m))
end
