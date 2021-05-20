#include "RemoveLoopMetadata.h"

using namespace std;
using namespace llvm;
using namespace team2_pass;

namespace team2_pass {

// to remove all loop metadata,
// remove all 'DILexicalBlockKind' metadata from all block terminators
PreservedAnalyses RemoveLoopMetadataPass::run(Module& M, ModuleAnalysisManager& MAM) {
    for(Function& F : M) {
        for(BasicBlock& BB : F) {
            Instruction *I = BB.getTerminator();
            I->setMetadata(Metadata::MetadataKind::DILexicalBlockKind, NULL);
        }
    }
    return PreservedAnalyses::all();
}

}