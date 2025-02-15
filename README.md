# Multi Threaded MAP REDUCE


## Usage
1. To build from source
```
gcc -o main src/main.c -Iinclude
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
