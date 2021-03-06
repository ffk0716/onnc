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
#ifndef BM188X_FUSE_OPTIMIZER_H
#define BM188X_FUSE_OPTIMIZER_H
#include "BM188xBackend.h"
#include "TGFuseOptimizer.h"
#include <onnc/Config/ONNX.h>

namespace onnc {

class BM188xFuseOptimizer : public TGFuseOptimizer
{
public:
  BM188xFuseOptimizer(TGBackend *pBackend)
      : TGFuseOptimizer(pBackend)
  {
    m_p1880backend = static_cast<BM1880Backend *>(pBackend);
  }

  ~BM188xFuseOptimizer() override = default;

  xNode *
  FuseConvScale(xGraph *pGraph, xNode *pConvNode, xNode *pScaleNode) override;

  xNode *FuseRelu(xGraph *pGraph, xNode *pNode, xNode *pReluNode) override;

private:
  BM1880Backend *m_p1880backend; // NOLINT
};

} // namespace onnc

#endif
