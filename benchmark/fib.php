<?php
function fib($n) {
  if ($n < 2)
    return $n;
  return fib($n - 1) + fib($n - 2);
}

for ($i = 0; $i < 5; $i++)
  echo fib(28) . "\n";
