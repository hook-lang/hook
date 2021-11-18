function fib(n) {
  if (n < 2)
    return n;
  return fib(n - 1) + fib(n - 2);
} 

const n = process.argv[2];
const m = process.argv[3];
for (let i = 0; i < n; i++)
  console.log(fib(m));
