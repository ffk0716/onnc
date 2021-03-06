//===- LoopIndexTensor.cpp ------------------------------------------------===//
//
//                             The ONNC Project
//
// See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
#include <onnc/IR/Compute/LoopIndexTensor.h>

using namespace onnc;

char LoopIndexTensor::ID = 0;

//===----------------------------------------------------------------------===//
// LoopIndexTensor
//===----------------------------------------------------------------------===//
LoopIndexTensor::LoopIndexTensor()
  : ComputeOperator("LoopIndexTensor", ID),
    m_Axis(0) {
}

LoopIndexTensor::LoopIndexTensor(const IntAttr& pAxis)
  : ComputeOperator("LoopIndexTensor", ID),
    m_Axis(pAxis) {
}

LoopIndexTensor::LoopIndexTensor(const LoopIndexTensor& pCopy)
  : ComputeOperator(pCopy) /* shallow copy */,
    m_Axis(pCopy.getAxis()) {
}

void LoopIndexTensor::print(std::ostream& pOS) const
{
  pOS << name() << "< " << getAxis() << ">";
}

bool LoopIndexTensor::classof(const ComputeOperator* pOp)
{
  if (nullptr == pOp)
    return false;
  return (pOp->getID() == &ID);
}
