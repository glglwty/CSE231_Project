//
// Created by glglwty on 2/24/17.
//

#include <iostream>
#include "231DFA.h"
#include "llvm/Pass.h"
#include "llvm/IR/InstVisitor.h"
using namespace llvm;
class ReachingInfo : public Info {
public:
    std::vector<unsigned> insts;
    void print() override {
        for (auto id : insts) {
            std::cerr << id << '|';
        }
        std::cerr << std::endl;
    }
    static bool equals(ReachingInfo * info1, ReachingInfo * info2) {
        assert(info1 && info2);
        return info1->insts.size() == info2->insts.size() &&
                std::equal(info1->insts.begin(), info1->insts.end(), info2->insts.begin());
    }
    static Info* join(ReachingInfo * info1, ReachingInfo * info2, ReachingInfo * result) {
        assert(info1 && info2 && result);
        assert(std::is_sorted(info1->insts.begin(), info1->insts.end()));
        assert(std::is_sorted(info2->insts.begin(), info2->insts.end()));
        result->insts.clear();
        std::set_union(info1->insts.begin(), info1->insts.end(), info2->insts.begin(), info2->insts.end(), std::back_inserter(result->insts));
        return result;
    }
};

class ReachingDefinitionAnalysis : public DataFlowAnalysis<ReachingInfo, true> {
public:
    static ReachingInfo ReachingNothing;
    ReachingDefinitionAnalysis(): DataFlowAnalysis(ReachingNothing, ReachingNothing) {}
    class Visitor: public InstVisitor<Visitor> {
        ReachingDefinitionAnalysis& r;
        void cb(Instruction &I) {
            unsigned id = r.getInstrToIndex()[&I];
            assert(info.insts.empty() || info.insts.back() != id);
            info.insts.emplace_back(id);
        }
    public:
        Visitor(ReachingDefinitionAnalysis& r) : r(r) {}
        ReachingInfo info;
        void visitBinaryOperator(BinaryOperator &I){
            cb(I);
        }
        void visitBranchInst(BranchInst &I) {

        }
        void visitSwitchInst(SwitchInst &I) {

        }
        void visitAllocaInst(AllocaInst &I) {
            cb(I);
        }
        void visitLoadInst(LoadInst     &I) {
            cb(I);
        }
        void visitStoreInst(StoreInst   &I) {

        }
        void visitGetElementPtrInst(GetElementPtrInst &I) {
            cb(I);
        }
        void visitICmpInst(ICmpInst &I) {
            cb(I);
        }
        void visitFCmpInst(FCmpInst &I) {
            cb(I);
        }
        void visitPHINode(PHINode       &I) {
            for (Instruction* p = &I; p && isa<PHINode>(p); p = p->getNextNode()) {
                cb(*p);
            }
        }
        void visitSelectInst(SelectInst &I) {
            cb(I);
        }
    };
private:
    void flowfunction(Instruction *I, std::vector<unsigned> &IncomingEdges, std::vector<unsigned> &OutgoingEdges,
                      std::vector<ReachingInfo *> &Infos) override {
        assert(I);
        //std::cout << "flow function in" << std::endl;
        Visitor v(*this);
        v.visit(I);
        //std::cout << "finished visiting" << std::endl;
        auto my_id = getInstrToIndex().at(I);
        //std::cout << "finished getting my id" << std::endl;
        for (auto income_node : IncomingEdges) {
            auto info = getEdgeToInfo().at(std::make_pair(income_node, my_id));
            assert(info);
            //std::cout << "old_info";
            //info->print();
            //std::cout << "merged into";
            //v.info.print();
            ReachingInfo new_out;
            //std::cout << "joining" << std::endl;
            ReachingInfo::join(info, &v.info, &new_out);
            v.info.insts = std::move(new_out.insts);
        }
        //std::cout << "finished gatering info" << std::endl;
        for (unsigned i = 0; i < OutgoingEdges.size(); i ++) {
            Infos.emplace_back(new ReachingInfo(v.info));
        }
        //std::cout << "flow function out" << std::endl;
    }
};

class ReachingDefinitionAnalysisPass : public FunctionPass {
public:
    static char ID;
    ReachingDefinitionAnalysisPass() : FunctionPass(ID) {}
    bool runOnFunction(Function &F) override {
        //std::cout << "begin running runonFunction" << std::endl;
        ReachingDefinitionAnalysis analysis;
        analysis.runWorklistAlgorithm(&F);
        //std::cout << "finished analysis" << std::endl;
        analysis.print();
        return false;
    }

};

ReachingInfo ReachingDefinitionAnalysis::ReachingNothing;
char ReachingDefinitionAnalysisPass::ID = 0;
static RegisterPass <ReachingDefinitionAnalysisPass> _("cse231-reaching", "part2reach",
                                                       true /* Only looks at CFG */,
                                                       true /* Analysis Pass */);
