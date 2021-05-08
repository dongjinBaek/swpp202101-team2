#!/bin/bash
if [ "$#" -ne 1 ]; then
  echo "run-filecheck.sh <FileCheck path>"
  exit 1
fi
echo "--- Start FileCheck.. ---"
set -e

PASSED=0
TOTAL=0
for f in `ls -1 filechecks` ; do
  echo "== filechecks/${f} =="
  ./bin/sf-compiler filechecks/${f} filechecks/${f}.s > filechecks/tmp.s
  $1 filechecks/${f} < filechecks/${f}.s
  if [ "$?" -eq 0 ]; then
    PASSED=$((PASSED+1))
  fi
  TOTAL=$((TOTAL+1))
  rm ./filechecks/${f}.s
done
rm filechecks/*.s
echo "Test passed: $PASSED / $TOTAL"