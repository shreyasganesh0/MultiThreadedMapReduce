# Multi Threaded MAP REDUCE


## Usage
1. To build from source
```
gcc -o main src/main.c -Iinclude

        OR
make clean
make
```

2. Build with script
```
./build.sh
```
    - Creates the program "combiner" in ./build/, run it with

3. Running the file. 
```
build/combiner S N < input.txt > output.txt
```
    - Where S and N are integers 
        - S- Number of slots per buffer
        - N - Number of reducers

## Troubleshooting
- The hashmap used is not dynamically increasing in size so the output may not fully display if the size set is inadequate

- Update the INITIAL_CAPACITY macro in the ./include/combiner.h file to be of adequate size (atleast double the number of max number of topics), default is 2000
