#include <llvm/Pass.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Function.h>
#include <llvm/Support/raw_ostream.h>
#include <llvm/IR/InstIterator.h>
#include <llvm/IR/IRBuilder.h>
#include <map>
#include <iostream>
#include <string>

using namespace llvm;

namespace {
  struct TestPass : public FunctionPass {
    static char ID;
    Module *m;

    TestPass() : FunctionPass(ID) {}

    bool doInitialization(Module &M) {
      m = &M;
      return false;
    }

    bool runOnFunction(Function &F) override {
      auto &context = F.getContext();
      auto F_updateBranchInfo = cast<Function>(m->getOrInsertFunction("updateBranchInfo",
                                                                      FunctionType::get(Type::getVoidTy(context),
                                                                                        {Type::getInt1Ty(context)},
                                                                                        false)));
      auto F_printOutBranchInfo = cast<Function>(
        m->getOrInsertFunction("printOutBranchInfo", FunctionType::get(Type::getVoidTy(context), false)));

      for (auto blk_it = F.begin(); blk_it != F.end(); ++blk_it) {
        auto &blk = *blk_it;
        if (auto binst = dyn_cast<BranchInst>(&blk.back())) {
          if (binst->isConditional()) {
            IRBuilder<>(binst).CreateCall(F_updateBranchInfo, {binst->getCondition()});
          }
        } else if (auto rinst = dyn_cast<ReturnInst>(&blk.back())) {
          IRBuilder<>(rinst).CreateCall(F_printOutBranchInfo);
        }
      }
      return true;
    }
  }; // end of struct TestPass
}  // end of anonymous namespace

char TestPass::ID = 0;
static RegisterPass <TestPass> X("cse231-bb", "project1section3",
                                 false /* Only looks at CFG */,
                                 false /* Analysis Pass */);
