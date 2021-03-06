//===- InitializePasses.h -------------------------------------------------===//
//
//                             The ONNC Project
//
// See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
#ifndef ONNC_CORE_INITIALIZE_PASSES_H
#define ONNC_CORE_INITIALIZE_PASSES_H

namespace onnc {

class PassRegistry;

/// Initialize all passes in Analysis library.
void InitializeAnalysis(PassRegistry&);

/// Initialize all pass options in Analysis library.
void InitializeAnalysisPassOptions();

//===----------------------------------------------------------------------===//
// Declarations of every single pass
//===----------------------------------------------------------------------===//
void* InitializeGraphLivenessAnalysisPass(PassRegistry&);
void* InitializeMemoryAllocationPass(PassRegistry&);
void* InitializeUpdateGraphOutputSizePass(PassRegistry&);
void* InitializeNodeIRSchedulerPass(PassRegistry&);

void InitializeUpdateGraphOutputSizePassOptions();

} // namespace of onnc

#endif
