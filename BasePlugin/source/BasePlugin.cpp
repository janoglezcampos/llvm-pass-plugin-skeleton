#include "BasePlugin.h"
#include <typeinfo>
#include "llvm/Support/CommandLine.h"

#include "llvm/IR/Constants.h"
#include "llvm/IR/IRBuilder.h"

using namespace std;
using namespace llvm;

namespace baseplugin
{
    // BasePluginPass implementations:
    PreservedAnalyses BasePluginPass::run(Module &M,
                                          ModuleAnalysisManager &AM)
    {
        outs() << "[INFO] : Analyzing module: " << M.getName() << "\n";
        return PreservedAnalyses::all();
    }

    PreservedAnalyses BasePluginPass::run(Function &F,
                                          FunctionAnalysisManager &AM)
    {
        outs() << "[INFO] : Analyzing function: " << F.getName() << "\n";

        for (auto &B : F)
            runOnBasicBlock(B);

        return PreservedAnalyses::all();
    }

    bool BasePluginPass::runOnBasicBlock(BasicBlock &B)
    {
        // Just an example to run something on every basic block
        return false;
    }

}