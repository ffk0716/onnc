//===---------------------------------------------------------------------===//
//
//                             The ONNC Project
//
// Copyright(c) 2018, The ONNC Team
//
// This file is part of the ONNC Project and is distributed under
// 3-clause BSD license (https://opensource.org/licenses/BSD-3-Clause)
//
// See LICENSE.TXT for details.
//
//===---------------------------------------------------------------------===//
#ifndef COMPUTE_OPERATOR_H
#define COMPUTE_OPERATOR_H
#include <onnc/IR/ONNXUtils.h>
#include <onnc/Support/IOStream.h>
#include <onnc/Config/ONNX.h>
#include <string>

namespace onnc {

using MemTable = std::map<std::string, uint64_t>;

enum class MemType { NEURON, WEIGHT };

struct MemOperand {
  std::string m_Name;
  uint64_t m_Addr;
  size_t m_Count;
  size_t m_Size;
  xTensorProtoDataType m_Type;
  MemType m_MemType;
  const xValue *m_Value;
  MemOperand(std::string pName, const xValue *pValue, MemType pMemType);
};

std::ostream &operator<<(std::ostream &pOS, const MemOperand &pMem);

class ComputeOperator2
{
public:
  ComputeOperator2(const xNode &pNode, const std::string &pTypeName);

  virtual ~ComputeOperator2() = default;

  // FIXME changed to use ENUM when compute ir finish
  const std::string &getTypeName() const { return m_TypeName; }

  const std::string &getLayerName() const { return m_LayerName; };

  std::vector<MemOperand *> &getMemOperands() { return m_MemOperands; };

  MemOperand *getMemOperand(unsigned int pIdx);

  const MemOperand *getMemOperand(unsigned int pIdx) const;

  virtual void emit() const = 0;

  virtual void memAlloc(MemTable &pPMemLayout);

  ComputeOperator2 *addMemOperand(MemOperand *pMemOperand);

private:
  std::string m_TypeName;
  std::string m_LayerName;

protected:
  std::vector<MemOperand *> m_MemOperands;
};

} // namespace onnc

#endif // COMPUTE_OPERATOR_H
