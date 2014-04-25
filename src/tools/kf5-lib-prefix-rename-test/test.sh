#!/bin/sh
set -e

rm -rf output
cp -r src output
cd output
echo "# Running"
../../kf5-lib-prefix-rename
cd ..
echo "# Results"
for file in expected/* ; do
    echo "## $file"
    diff -u $file output/$(basename $file)
done
