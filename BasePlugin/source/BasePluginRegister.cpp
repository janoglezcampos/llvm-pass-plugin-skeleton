#include "llvm/Passes/PassPlugin.h"
#include "llvm/Passes/PassBuilder.h"
#include "BasePlugin.h"

__declspec(dllexport) extern "C" ::llvm::PassPluginLibraryInfo LLVM_ATTRIBUTE_WEAK
    llvmGetPassPluginInfo()
{
    return {
        LLVM_PLUGIN_API_VERSION,
        "BasePluginPass", "v0.1",
        [](PassBuilder &PB)
        {
            // Allows to run the pass alone, only works with opt.exe
            // Enables: opt -load-pass-plugin="<whatever>/LLVMBasePlugin.dll" -passes="base-plugin-pass"
            PB.registerPipelineParsingCallback(
                [](StringRef Name, ModulePassManager &MPM,
                   ArrayRef<PassBuilder::PipelineElement>)
                {
                    if(Name == "base-plugin-pass"){
                        MPM.addPass(baseplugin::BasePluginPass());
                        return true;
                    }
                    return false; });

            // Allows to run the pass as part of the defaults optimization passes
            // Enables: clang -O0 -Xclang -disable-O0-optnone -fpass-plugin="<whatever>/LLVMBasePlugin.dll" ...
            // Enables: opt -O0 -load-pass-plugin="<whatever>\LLVMBasePlugin.dll" ...

            // This extension point allows adding optimization passes after most of the
            // main optimizations, but before the last cleanup-ish optimizations.
            /*PB.registerScalarOptimizerLateEPCallback(
                [](FunctionPassManager &FPM, OptimizationLevel opt)
                {
                    FPM.addPass(baseplugin::BasePluginPass());
                }); */
            PB.registerFullLinkTimeOptimizationEarlyEPCallback(
                [](ModulePassManager &MPM, OptimizationLevel opt)
                {
                    MPM.addPass(baseplugin::BasePluginPass());
                });
        }};
}