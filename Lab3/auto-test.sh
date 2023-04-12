for file in ./test/*.cmm
    do
        echo Testing with $file
        ./parse.sh $file
    done