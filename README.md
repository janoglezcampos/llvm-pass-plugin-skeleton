# LLVM optimization pass skeleton

This is a detailed step-by-step description on how to compile out-of-tree a dynamically linked LLVM pass plugin for the new pass manager in Windows x86_64, tested with llvm 16.x

Create a new directory where you prefer called "llvm".
Enter the new directory and clone the LLVM source code.
Create an aditional folder inside "llvm" called "llvm-install".
Create an aditional folder inside "llvm" called "llvm-build".

Doing this from powershell requires git (note that you can choose a newer release branch, or dev branch when cloning llvm, in this case im using release/16.x):

```
mkdir llvm; mkdir llvm/llvm-install; mkdir llvm/llvm-build
cd llvm
git clone -b "release/16.x" https://github.com/llvm/llvm-project
```

Now we need to build LLVM, this includes its libraries, header files, configurations... that we will need to compile our pass plugin later, if you prefer, [LLVM has its own tutorial on this](https://llvm.org/docs/CMake.html).

To do this we need some tools. If you already have CMAKE, a build system and a prefered compiler installed then you can ignore this, but I want to specify everything like if we were in a new computer starting from 0.

What we are gonna see are my personal preferences of tools and installation methods, but you can use any other, and also install them through their installers or released binaries. We will install CMAKE, Ninja as our build system, Clang as our compiler and LLD as linker through MSYS2.

First download and install [MSYS2](https://www.msys2.org/).
Now launch the MSYS2 MSYS terminal (Base enviroment of MSYS)

Install the following packages using pacman (pacman -S \<package name\>):
```
    pacman -S mingw-w64-x86_64-cmake
    pacman -S mingw-w64-x86_64-ninja (is a dependency of cmake, so should already be installed)
    pacman -S mingw-w64-x86_64-clang
    pacman -S mingw-w64-x86_64-lld
```
You can close the MSYS2 terminal already. (This is because I like to use the windows terminal, if you prefer, you could keep using MSYS2 and skip the next step)

All the required bynaries should be at \<msys2 installation folder\>/mingw64/bin, (by default C:/msys64//mingw64/bin), so if its not already on your PATH, you need to add this directory ([HOW TO](https://docs.oracle.com/en/database/oracle/machine-learning/oml4r/1.5.1/oread/creating-and-modifying-environment-variables-on-windows.html#GUID-DD6F9982-60D5-48F6-8270-A27EC53807D0)).

Now its time to configure our project, go inside the llvm-build directory previously created and run:
```
cmake -G Ninja \
    -DLLVM_ENABLE_PLUGINS=On \
    -DCMAKE_INSTALL_PREFIX="../llvm-install" \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_C_COMPILER=clang \
    -DCMAKE_CXX_COMPILER=clang++ \
     -DLLVM_USE_LINKER=lld \
    ../llvm-project/llvm
```
If everything went correctly, the build directory should have been populated with all the required files to start the actual build, so now we just need to run the following (this will take a while, so you better find something to do xd, to speed this up, you can increase the number of parallel compilations to 8 with: `-- -j 8`. You can increase it even further if you feel like):

```
cmake --build . [-- -j 8]
```

Now we will move everything we will need to the llvm-install directory with:

```
cmake --build . --target install
```

This should have created some important folder inside the llvm-install directory:
* llvm-install/bin: LLVM binaries.
* llvm-install/lib: LLVM libraries for static linking.
* llvm-install/include: LLVM header files.

You can add "llvm-install/bin" to your PATH, since it contains some tools that you will end up using like opt, llc, llvm-dis... (Take care with any previous installation of clang, either with msys or in any other way. It will not break, but may confuse you since only the first appearance of the executable will be used).

# Building this pass
At this point, we have all the requirements to be able to compile and run our pass plugins, so clone this repository to anywere you like.


```
git clone \<this repository>
```

Enter the directory, create a new folder called build (equivalent to llvm-build folder previously created) and enter it.

```
cd \<this reposity>
mkdir build; cd build
```

Now, as we did before, lets configure it. For this, we just need to remember one path: \<path to llvm-install directory>/lib/cmake/llvm, that will tell CMAKE were all the include files and aditional libraries can be found.

```
cmake -G Ninja -DLLVM_DIR=<path to llvm-install directory>\lib\cmake\llvm -DCMAKE_C_COMPILER=clang -DCMAKE_CXX_COMPILER=clang++ -DCMAKE_BUILD_TYPE=Release ./..

```

Once configured, just build:

```
cmake --build . -- -j 8
```

At this point, you probably saw a warning about an unmatched definition of llvmGetPassPluginInfo() between the source code of this repo and the llvm headers. This is because the original llvm header file defining the function doesnt include "__declspec(dllexport)", and I found this i a must in windows. The reason behind is that by default, at least in my tests, the compiler wont export the function in the final DLL, and it is required so the opt tool can find it at load time. Now you have to options, just ignore it, or modify the original header file, this is up to you.

The file should be found at \<this repo>/build/BasePlugin/LLVMBasePlugin.dll

# Running you pass
To be able to run this pass, I think is important to have a quick review about how the build process goes with LLVM.
Mainly, for the understanding of this post, our code will go through the following stages:

1. **IR Generation:** The intermediate representations is a low level representation of our code, common to all programming lenguages and any machine type.
2. **IR Optimization:** The IR is modified through passes, each one changing the content of the IR. Here we will insert our pass.
3. **MIR Generation:** The Machine Intermediate representation is similar to the IR (in fact, is a tranformation of it), but now using passes specific to the machine type we are targeting. Writting passes for the MIR generation pipeline would allow applying modifications to the actual instructions used, so this is the point if we would like to do such.
4. **Object file generation:** Generate object files from MIR.
5. **Linking: Create the final** executable by linking the object files.

This is not always true, or exact, and the order may vary. For example, when applying linking time optimizations, the linker takes both the external already compiled libraries, and our IR, to perform analysis on all the files, before applying some especific IR optimizations. We need to understand this is a complex process, but I think having this general idea is enough for now. Also, normally we dont see the hole process, since clang is capable of handling everything by itself, but here we will see the specific tools to run each step.

Once we know were we want to apply our optimizations (step 2), we need to get there.

To generate the IR files, force clang to emit the IR for every file (note that we can either generate it with .ll extension, wich is a readable representation of the IR, or .bc, wich is a binary representation), without optimizations (by default, -O0 will add a optnone tag to every function, telling the optimizer to ignore it. Since we dont want this behaviour, we use -Xclang -disable-O0-optnone).

Enter the pass-test directory and run:

```
clang -O0 -Xclang -disable-O0-optnone -S -emit-llvm ./source/main.c ./source/utils.c
```

After that, we have to options, either keep working with every individual file, or merge all of them into a single one, we will choose the second, so lets call llvm-link.
```
llvm-link ./main.ll ./utils.ll -S -o test.ll
```

Now the moment we were waiting, lets invoke opt, the llvm IR transformator, and give it our pass. Also, we can run any optimization we want, in the order we prefer, including the default pipelines O1, O2... By just calling opt over the generated files over and over.

```
opt -load-pass-plugin="<path to the pass dll>" -passes="base-plugin-pass" test.ll -o test.op.ll
```

And thats it, our pass did its job and now all that remains is generate our final binary.
To generate object files from the IR files, we use the llvm compiler: llc; llc wil internally run all the machine dependent passes converting the IR into MIR, to finally transform it, in this case to x86 code. It is possible to stop llc in the middle of the process to emmit the MIR, but we will skip it in favor of simplicity. Also, another cool trick is that we can force llc to emmit readable assembly, wich is pretty cool if we want to check the actual output withou a decompiler.

```
llc --mtriple=x86_64-pc-windows-msvc -filetype=obj  ./test.op.ll -o test.obj
```

Finally link the object files together using the linker of your preference. In this case, as we are talking about llvm lets use lld through clang (remember clang is not a linker, but is capable of calling lld by itself, and also specify the default dependency paths).

```
clang ./test.obj -o test.exe
```

And there it is, the final binary, rigth out of the oven.
Now you have all that you need to start messing around, soon Ill be posting about how to write obfuscators with LLVM, so this will serve as the starting place.

Wish you luck on your travels to the inners of LLVM :)
