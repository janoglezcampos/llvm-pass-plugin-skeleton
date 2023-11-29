#ifndef LLVM_BASEPLUGIN_H
#define LLVM_BASEPLUGIN_H

#include "llvm/IR/PassManager.h"

using namespace llvm;

namespace baseplugin
{
    struct BasePluginPass : public PassInfoMixin<BasePluginPass>
    {
    public:
        PreservedAnalyses run(Module &M, ModuleAnalysisManager &AM);
        PreservedAnalyses run(Function &F, FunctionAnalysisManager &AM);

    private:
        bool runOnBasicBlock(BasicBlock &B);
    };

} // namespace llvm

#endif // LLVM_TRANSFORMS_UTILS_HELLOWORLD_H
