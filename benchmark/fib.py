import sys

def fib(n):
  if n < 2: return n
  return fib(n - 1) + fib(n - 2)

n = int(sys.argv[1])
m = int(sys.argv[2])
for i in range(0, n):
  print(fib(m))
