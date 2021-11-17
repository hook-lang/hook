#!/usr/bin/env bash

printf "Running benchmark...\n\n"

echo "Hook"
hook --version
start=$(date +%s%N)
hook benchmark/fib.hook
hook_elapsed=$((($(date +%s%N) - $start) / 1000000))
echo ""

echo "Node.js"
node --version
start=$(date +%s%N)
node benchmark/fib.js
nodejs_elapsed=$((($(date +%s%N) - $start) / 1000000))
echo ""

echo "Lua"
lua -v
start=$(date +%s%N)
lua benchmark/fib.lua
lua_elapsed=$((($(date +%s%N) - $start) / 1000000))
echo ""

echo "PHP"
php --version
start=$(date +%s%N)
php benchmark/fib.php
php_elapsed=$((($(date +%s%N) - $start) / 1000000))
echo ""

echo "Python 3"
python3 --version
start=$(date +%s%N)
python3 benchmark/fib.py
python3_elapsed=$((($(date +%s%N) - $start) / 1000000))
echo ""

echo "Ruby"
ruby --version
start=$(date +%s%N)
ruby benchmark/fib.rb
ruby_elapsed=$((($(date +%s%N) - $start) / 1000000))
echo ""

printf "Hook      | Node.js  | Lua      | PHP      | Python 3 | Ruby\n";
echo "----------+----------+----------+----------+----------+----------";
fmt="%-7s   | %-6s   | %-6s   | %-6s   | %-6s   | %s\n\n"
printf "$fmt" "$hook_elapsed ms" "$nodejs_elapsed ms" "$lua_elapsed ms" "$php_elapsed ms" "$python3_elapsed ms" "$ruby_elapsed ms"
