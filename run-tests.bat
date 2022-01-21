@echo off

echo "Running tests.."

set /a n=0
for %%f in (test\*.hook) do (
  bin\hook %%f
  set /a n=n+1
)

echo "%n% test(s)"
