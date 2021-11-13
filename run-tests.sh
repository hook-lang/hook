#!/usr/bin/env bash

echo "Running tests..."

n=0
for f in test/*.hook ; do
  bin/hook $f
  n=$(($n + 1))
done

echo "$n test(s)"
