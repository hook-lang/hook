function fib(n) {
  if (n < 2)
    return n;
  return fib(n - 1) + fib(n - 2);
} 

for (let i = 0; i < 5; i++)
  console.log(fib(28));
