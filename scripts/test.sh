#!/usr/bin/env bash

echo "Running tests.."

n=0
for f in tests/*.hk ; do
  bin/hook $f
  n=$(($n + 1))
done

echo "$n test(s)"
