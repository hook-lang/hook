@echo off

echo "Running tests.."

set /a n=0
for %%f in (tests\*.hk) do (
  bin\hook %%f
  set /a n=n+1
)

echo "%n% test(s)"
