<?php
function fib($n) {
  if ($n < 2)
    return $n;
  return fib($n - 1) + fib($n - 2);
}

$n = $argv[1];
$m = $argv[2];
for ($i = 0; $i < $n; $i++)
  echo fib($m) . "\n";
