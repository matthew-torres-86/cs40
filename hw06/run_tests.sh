#! /bin/sh
testFiles=$(ls $2 *.um)
echo $testFiles
for testFile in $testFiles ; do
    testName=$(echo $testFile | sed -E 's/(.*).um/\1/')
    $(echo umdump $testFile) > $testName.dump
done

