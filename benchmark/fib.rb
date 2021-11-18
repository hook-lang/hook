def fib(n)
  if n < 2 then
    return n
  end
  fib(n - 1) + fib(n - 2)
end

n = ARGV[0].to_i
m = ARGV[1].to_i
for i in 0...n
  puts fib(m)
end
