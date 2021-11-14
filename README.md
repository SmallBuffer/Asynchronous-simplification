# Asynchronous simplification:</br>
## ASYSIM Tool: 
The tool can convert executable asynchronous code into equivalent synchronous code, thus simplifying the verification of asynchronous code.</br>
We provide concrete implementations of several examples and their corresponding conversion results in the "**examples**" directory.</br>
The tool first uses clang ASTMatchers to tailor the asynchronous code to generate an asynchronous intermediate structure, and then transforms it into a synchronous intermediate structure through the conversion algorithm, and finally restores the transformed intermediate structure to obtain the equivalent synchronous code.</br>
## Conversion algorithm and intermediate structure: 
We independently provide our conversion algorithm source code (trans_algorithm.cpp) in the "**middle_construction**" directory. In the "**middle_construction/Input**" directory, we provide the intermediate structure of the asynchronous code to be converted in the above example. After inputting them into the conversion algorithm, The synchronized intermediate structure after conversion will be obtained (you can view it in the "**middle_construction/OutputBeforeTransform**" folder), and then the conversion algorithm will further convert the data exchange part of the obtained intermediate structure into equivalent assignment statements and output (you can view under "**middle_construction/OutputAfterTransform**" folder)
## How to build:
The tools are built using LibTooling and LibASTMatchers. The specific construction process is as follows:</br>
### **Obtain llvm and clang**
<ul>
<li>Get llvm.src</li>
<li>Get clang.src and put it in llvm/tools/clang</li>
<li>Get clang-tools-extra.src and put it in llvm/tools/clang/tools/extra</li>
</ul>
These are available on the GitHub release <a href="https://github.com/llvm/llvm-project/releases/tag/llvmorg-11.1.0">page</a>.

### **Create ASYSIM tool**
```javascript
cd llvm/tools/clang
mkdir tools/extra/ASYSIM
echo'add_subdirectory(ASYSIM)' >> tools/extra/CMakeLists.txt
```
Put ASYSIM.cpp and CMakeLists.txt under tools/extra/ASYSIM</br>

### **Use CMake to build, it is recommended to choose build out of source**
Create a new llvm-build folder outside the llvm folder as our build directory
```javascript
mkdir llvm-build
cd llvm-build
cmake /path/to/llvm.src
cmake --build.
```
After the build is complete, you can find the executable ASYSIM in the generated "**llvm-build/bin**"

### **Run the ASYSIM tool**
```javascript
./ASYSIM /path/to/Server.cpp /path/to/Client.cpp --
```
The tool will output the conversion result (result.cpp) in the current directory by default, and you can also specify the output path through the "-p" parameter:</br>
```javascript
./ASYSIM /path/to/example.cpp -p /path/to/output/result.c --
```
We provide the synchronization programs generated by the above examples transformation in the **"examples/OutputAfterTransform"** directory .

## Run the converted synchronization program: 
The converted synchronization program contains multi-threaded code based on linux system, so it only supports linux system to compile and run. Windows 10 users can compile and run in WSL2. The static library libpthread.a needs to be linked when compiling, so the -lpthread parameter should be added during compilation: 
```javascript
gcc -pthread -o result result.c
```

## Tool limitations: 
If the source code is too complicated to transform, a small number of variable, structure or class declarations may be missing after the transformation is generated, which needs to be added manually
In addition, in order to facilitate the conversion, the string type in the asynchronous code to be converted is required to adopt the "std:string" type uniformly, and if there are multiple rounds, the round variable needs to be declared in the source code. 


# Use duet to verify the c program:
In the computer equipped with duet software (Linux operating system), start the system terminal under the duet folder and enter the command:</br>
<b><I>./duet.native -proofspace &lt;c file relative path &gt;</I></b></br>
You can get the verification result</br>
