//===- Concat.cpp ---------------------------------------------------------===//
//
//                             The ONNC Project
//
// See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
#define DEBUG_TYPE "tg_concat"
#include "Concat.h"
#include "../BM188xVisitor.h"

using namespace onnc;
using namespace onnc::BM188X;

char BM188X::Concat::ID = 0;

//===----------------------------------------------------------------------===//
// Concat
//===----------------------------------------------------------------------===//
BM188X::Concat::Concat(const IntAttr& pAxis)
    : onnc::Concat(pAxis), m_NeedQuantizeNum(0), m_RShiftWidth(),
      m_ThresholdXQuantized()
{
  setID(ID);
}

BM188X::Concat::~Concat()
{
}

void BM188X::Concat::setRShiftWidth(unsigned int pIdx, int pValue)
{
  if (m_RShiftWidth.size() > pIdx)
    m_RShiftWidth[pIdx] = pValue;
}

const std::vector<int>& BM188X::Concat::getRShiftWidth() const
{
  return m_RShiftWidth;
}

void BM188X::Concat::setThresholdXQuantized(unsigned int pIdx, int pValue)
{
  if (m_ThresholdXQuantized.size() > pIdx)
    m_ThresholdXQuantized[pIdx] = pValue;
}

const std::vector<int>& BM188X::Concat::getThresholdXQuantized() const
{
  return m_ThresholdXQuantized;
}

void BM188X::Concat::print(std::ostream& pOS) const
{
  // TODO
}

void BM188X::Concat::accept(ComputeVisitor& pV)
{
  BM188xVisitor* visitor = dyn_cast<BM188xVisitor>(&pV);
  if (nullptr != visitor)
    visitor->visit(*this);
}

void BM188X::Concat::accept(ComputeVisitor& pV) const
{
  BM188xVisitor* visitor = dyn_cast<BM188xVisitor>(&pV);
  if (nullptr != visitor)
    visitor->visit(*this);
}

bool BM188X::Concat::classof(const ComputeOperator* pOp)
{
  if (nullptr == pOp)
    return false;
  return (pOp->getID() == &ID);
}
