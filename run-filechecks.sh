#!/bin/bash
if [ "$#" -ne 1 ]; then
  echo "run-filecheck.sh <FileCheck path>"
  exit 1
fi
echo "--- Start FileCheck.. ---"
set -e

PASSED=0
TOTAL=0
mkdir -p filechecks-out
for f in `ls -1 filechecks` ; do
  echo "== filechecks/${f} =="
  ./bin/sf-compiler filechecks/${f} filechecks-out/${f}.s -p ${f%_*} > filechecks-out/tmp.s
  $1 filechecks/${f} < filechecks-out/${f}.s
  if [ "$?" -eq 0 ]; then
    PASSED=$((PASSED+1))
  fi
  TOTAL=$((TOTAL+1))
done
rm -r filechecks-out
echo "Test passed: $PASSED / $TOTAL"