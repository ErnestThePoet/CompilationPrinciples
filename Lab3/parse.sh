mkdir -p out
file_name=$(basename $1)
./build/parser $1 > ./out/${file_name}_out.txt 2>&1