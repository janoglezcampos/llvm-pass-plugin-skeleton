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

What we are gonna see are my personal preferences of tools and installation methods, but you can use any other, and also install them throw their installers or released binaries. We will install CMAKE, Ninja as our build system, Clang as our compiler and LLD as linker throw MSYS2.

First download and install [MSYS2](https://www.msys2.org/).
Now launch the MSYS2 MSYS terminal (Base enviroment of MSYS)

Install the following packages using pacman (pacman -S \<package name\>):
    pacman -S mingw-w64-x86_64-cmake
    pacman -S mingw-w64-x86_64-ninja (is a dependency of cmake, so should already be installed)
    pacman -S mingw-w64-x86_64-clang
    pacman -S mingw-w64-x86_64-lld

You can close the MSYS2 terminal already.

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

You can add "llvm-install/bin" to your PATH, since it contains some tools that you will end up using like opt, llc, llvm-dis...
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
cmake -G Ninja -DLLVM_DIR=C:\Pentest\Develop\llvm-project\build\lib\cmake\llvm -DCMAKE_C_COMPILER=clang -DCMAKE_CXX_COMPILER=clang++ -DLLVM_USE_LINKER=lld -DCMAKE_BUILD_TYPE=Release ./..

```

Once configured, just build:

```
cmake --build . -- -j 8
```

clang -O0 -Xclang -disable-O0-optnone -c -emit-llvm .\hello.c -o hello.bc
clang -O0 -Xclang -disable-O0-optnone -S -emit-llvm .\hello.c -o hello.ll

 clang -O0 -Xclang -disable-O0-optnone -fpass-plugin="C:\Users\httpyxel\Dev\llvm\llvm-passes\llvm-pass-plugin-skeleton\build\BasePlugin\LLVMBasePlugin.dll" .\hello.c -o hello.exe
clang -O0 -g -fpass-plugin="C:\Users\httpyxel\Dev\llvm\llvm-passes\llvm-pass-plugin-skeleton\build\BasePlugin\LLVMBasePlugin.dll" hello.c -o hello.exe  
opt -load-pass-plugin="C:\Users\httpyxel\Dev\llvm\llvm-passes\llvm-pass-plugin-skeleton\build\BasePlugin\LLVMBasePlugin.dll" -passes="base-plugin-pass" hello.bc -o hello.op.bc

llc --mtriple=x86_64-pc-windows-msvc -start-after=x86-isel -stop-after=irtranslator hello.mir -o hello.late.mir -debug-pass=Execution  

llc --mtriple=x86_64-pc-windows-msvc hello.bc -filetype=obj -o hello.obj