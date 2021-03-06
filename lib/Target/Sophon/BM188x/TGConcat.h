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
#ifndef ONNX_BM1880_TGCONCAT_H
#define ONNX_BM1880_TGCONCAT_H
#include "BM188xComputeOperator.h"
#include <onnc/Target/Sophon/BM188x/common_calibration2.pb.h>
#include <onnc/Config/ONNX.h>

namespace onnc {
namespace BM188X {

// m_MemOperands: input, output
class TGConcat : public BM188xComputeOperator
{
public:
  TGConcat(const xNode &pNode);

  void emit() const override;
  TGConcat *addMemOperands(std::vector<MemOperand *> &pInput,
                           MemOperand *pOutput);
  void
  update(const tg::bm1880::LayerCalibrationParameter *pLayerCtable) override;

private:
  std::vector<unsigned long int> m_InputAddr;
  std::vector<int> m_InputDims;
  std::vector<int> m_OutputDim;
  int m_ConcatAxis;
  int m_NeedQuantizeNum;
  std::vector<int> m_RShiftWidth;
  std::vector<int> m_ThresholdXQuantized;
};

} // namespace BM188X
} // namespace onnc

#endif
