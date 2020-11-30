#! /bin/sh

cd tests
rm *Fail 2> /dev/null
testFiles=$(ls *.um)
  
for testFile in $testFiles ; do
    testName=$(echo $testFile | sed -E 's/(.*).um/\1/') 
    inputFileName=$testName\.0
    inputFile=/dev/null
    if [ -f $inputFileName ] ; then
        inputFile=$inputFileName 
    fi
    .././um $testFile < $inputFile > tmp
    outputFile=$testName\.1
    if [ -f $outputFile ] ; then
        if cmp -s "tmp" "$outputFile" ; then
            echo passed $testName
        else
            echo !failed $testName!
            echo diff of two outputs:
            echo $(diff tmp $outputFile)
            cp tmp $testName\Fail
        fi
    else
        echo passed $testName
    fi
    rm tmp
done
