//
// Created by glglwty on 3/17/17.
//

#include <iostream>
#include <unordered_map>
#include <iterator>
#include <set>
#include "231DFA.h"
#include "llvm/Pass.h"
#include "llvm/IR/InstVisitor.h"

using namespace llvm;

struct LivenessInfo : public Info {
    std::set<unsigned> s;
    void print() override {
        std::copy(s.begin(), s.end(), std::ostream_iterator<unsigned>(std::cerr, "|"));
        std::cerr << std::endl;
    }

    static bool equals(const LivenessInfo *info1, const LivenessInfo *info2) {
        return info1->s.size() == info2->s.size() && std::equal(info1->s.begin(), info1->s.end(), info2->s.begin());
    }

    static Info *join(const LivenessInfo *info1, const LivenessInfo *info2, LivenessInfo *result) {
        result->s.clear();
        std::set_union(info1->s.begin(), info1->s.end(), info2->s.begin(), info2->s.end(), std::inserter(result->s, result->s.begin()));
        return result;
    }
};

class LivenessAnalysis : public DataFlowAnalysis<LivenessInfo, false> {
    static LivenessInfo Bottom;
    class Visitor: public InstVisitor<Visitor> {
        LivenessAnalysis& r;
        std::vector<LivenessInfo *> &Infos;
        const std::vector<unsigned> &OutgoingEdges;

        void remove_all(unsigned id) {
            for (auto i : Infos) {
                i->s.erase(id);
            }
        }
        void value_instruction(Instruction &I) {
            visitInstruction(I);
            remove_all(r.getInstrToIndex()[&I]);
        }
    public:
        Visitor(LivenessAnalysis& r, std::vector<LivenessInfo *> &Infos, const std::vector<unsigned> &OutgoingEdges) : r(r), Infos(Infos), OutgoingEdges(OutgoingEdges) {}
        void visitInstruction(Instruction &I) {
            for (auto op : I.operand_values()) {
                if (auto instr = dyn_cast<Instruction>(op)) {
                    auto id = r.getInstrToIndex()[instr];
                    for (auto i : Infos) {
                        i->s.emplace(id);
                    }
                }
            }
        }
        void visitBinaryOperator(BinaryOperator &I){
            value_instruction(I);
        }
        void visitAllocaInst(AllocaInst &I) {
            value_instruction(I);
        }
        void visitLoadInst(LoadInst     &I) {
            value_instruction(I);
        }
        void visitGetElementPtrInst(GetElementPtrInst &I) {
            value_instruction(I);
        }
        void visitICmpInst(ICmpInst &I) {
            value_instruction(I);
        }
        void visitFCmpInst(FCmpInst &I) {
            value_instruction(I);
        }
        void visitPHINode(PHINode       &I) {
            for (Instruction* p = &I; p && isa<PHINode>(p); p = p->getNextNode()) {
                remove_all(r.getInstrToIndex()[p]);
            }
            for (Instruction* p = &I; p && isa<PHINode>(p); p = p->getNextNode()) {
                auto phi = cast<PHINode>(p);
                for (unsigned i = 0; i < phi->getNumIncomingValues(); i ++) {
                    if (auto in_instr = dyn_cast<Instruction>(phi->getIncomingValue(i))) {
                        for (unsigned j = 0; j < OutgoingEdges.size(); j++) {
                            if (r.getIndexToInstr()[OutgoingEdges[j]]->getParent() == phi->getIncomingBlock(i)) {
                                Infos[j]->s.emplace(r.getInstrToIndex()[in_instr]);
                            }
                        }
                    }
                }
            }
        }
        void visitSelectInst(SelectInst &I) {
            value_instruction(I);
        }
    };
    void flowfunction(Instruction *I, std::vector<unsigned> &IncomingEdges, std::vector<unsigned> &OutgoingEdges,
                      std::vector<LivenessInfo *> &Infos) override {
        LivenessInfo input_info;
        auto my_id = getInstrToIndex().at(I);
        for (auto income_node : IncomingEdges) {
            LivenessInfo in_info_new;
            LivenessInfo::join(getEdgeToInfo().at(std::make_pair(income_node, my_id)), &input_info, &in_info_new);
            input_info = std::move(in_info_new);
        }
        for (unsigned i = 0; i < OutgoingEdges.size(); i ++) {
            Infos.emplace_back(new LivenessInfo(input_info));
        }
        Visitor(*this, Infos, OutgoingEdges).visit(I);
    }
public:
    LivenessAnalysis() : DataFlowAnalysis(Bottom, Bottom) {}

};

class LivenessAnalysisPass : public FunctionPass {
public:
    static char ID;

    LivenessAnalysisPass() : FunctionPass(ID) {}

    bool runOnFunction(Function &F) override {
        //std::cout << "begin running runonFunction" << std::endl;
        LivenessAnalysis analysis;
        analysis.runWorklistAlgorithm(&F);
        //std::cout << "finished analysis" << std::endl;
        analysis.print();
        return false;
    }

};

LivenessInfo LivenessAnalysis::Bottom;
char LivenessAnalysisPass::ID = 0;
static RegisterPass<LivenessAnalysisPass> _("cse231-liveness", "part3live",
                                            true /* Only looks at CFG */,
                                            true /* Analysis Pass */);