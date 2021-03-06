//===- LivenessAnalysisTest.cpp -------------------------------------------===//
//
//                             The ONNC Project
//
// See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
#include <skypat/skypat.h>
#include <onnc/Analysis/LivenessAnalysis.h>
#include <onnc/Core/PassManager.h>
#include <onnc/Support/OStrStream.h>
#include <onnc/Support/IOStream.h>
#include <onnc/Config/ONNX.h>

using namespace onnc;
using namespace std;

namespace {
  using TP_DataType = xTensorProtoDataType;
  const TP_DataType TP_FLOAT = (xTensorProtoDataType)xValueType::kFloat;

  vector<xDimension> intSizeToDim(const vector<int> &pSizes)
  {
    vector<xDimension> dims;
    for (int d : pSizes)
      dims.push_back(xDimension(d));
    return dims;
  }

  class GraphHelper;

  class NodeHelper
  {
  public:
    NodeHelper(GraphHelper *pParent, xNode *pNode)
      : m_Parent(pParent), m_Node(pNode), m_HasInitFirstOutput(false) {}
    NodeHelper()
      : m_Parent(nullptr), m_Node(nullptr), m_HasInitFirstOutput(false) {}

    NodeHelper & addOutput(const string &pName, const vector<int> &pSizes,
                           TP_DataType pElemTy = TP_FLOAT);
  private:
    GraphHelper *m_Parent;
    xNode *m_Node;
    bool m_HasInitFirstOutput;
  };

  class GraphHelper
  {
  public:
    GraphHelper(xGraph *pGraph = nullptr)
    {
      m_Graph = pGraph;
      m_NeedDeleteGraph = false;
      if (!pGraph) {
        m_NeedDeleteGraph = true;
        m_Graph = new xGraph;
      }
    }
    virtual ~GraphHelper()
    {
      if (m_NeedDeleteGraph)
        delete m_Graph;
    }

    xValue & addInput(const string pName,
                           const vector<int> &pSizes /* nchw */,
                           TP_DataType pElemType = TP_FLOAT)
    {
      xValue *val = m_Graph->addInput();
      val->setUniqueName(pName)
         ->setSizes(intSizeToDim(pSizes))
         ->setElemType(pElemType);

      m_Values[pName] = val;
      return *val;
    }

    // returned Node is only valid before another addNode().
    NodeHelper & addNode(const string &pName, const vector<string> pInputs)
    {
      xNode *n = m_Graph->create(xSymbol(pName));
      for (auto & input : pInputs) {
        assert(m_Values.find(input) != m_Values.end() &&
               "Value is not found.");
        n->addInput(m_Values[input]);
      }
      m_Nodes[pName] = NodeHelper(this, n);
      m_Graph->appendNode(n);
      return m_Nodes[pName];
    }

    void addInitializer(const string &pName,
                        TP_DataType pElemType = TP_FLOAT)
    {
      xTensor t;
      t.elem_type() = (xTensorProtoDataType)xValueType::kFloat;
      t.setName(pName);
      m_Graph->addInitializer(t, pName);
    }

    void addValue(xValue *pValue)
    {
      auto it = m_Values.find(pValue->uniqueName());
      assert((it == m_Values.end() || it->second == pValue) &&
             "Try to add different value with same name.");
      if (it == m_Values.end())
        m_Values[pValue->uniqueName()] = pValue;
    }

    void finish(const vector<string> pOutputs)
    {
      for (auto & output : pOutputs) {
        assert(m_Values.find(output) != m_Values.end() &&
               "Value is not found.");
        // register graph output
        m_Graph->registerOutput(m_Values[output]);
      }
    }

    xGraph *getGraph() { return m_Graph; }

  private:
    unordered_map<string, NodeHelper> m_Nodes;
    unordered_map<string, xValue *> m_Values; 
    bool m_NeedDeleteGraph;
    xGraph *m_Graph;
  };

  NodeHelper & NodeHelper::addOutput(
    const string &pName, const vector<int> &pSizes,
    TP_DataType pElemTy)
  {
    assert(m_Node && "xNode should be created before any operation.");
    xValue *out;
    if (m_HasInitFirstOutput)
      out = m_Node->addOutput();
    else {
      out = m_Node->output();
      m_HasInitFirstOutput = true;  
    }

    out->setUniqueName(pName)
        ->setSizes(intSizeToDim(pSizes))
        ->setElemType(pElemTy);

    m_Parent->addValue(out);

    return *this;
  }

}

static const char *testAlexNetAnswer =
  "conv1_w_0 [0, 0]\ndata_0 [0, 0]\nconv1_b_0 [0, 0]\n"
  "conv1_1 [0, 1]\nconv1_2 [1, 2]\nnorm1_1 [2, 3]\n"
  "pool1_1 [3, 4]\nconv2_1 [4, 5]\nconv2_b_0 [4, 4]\nconv2_w_0 [4, 4]\n"
  "conv2_2 [5, 6]\n"
  "norm2_1 [6, 7]\npool2_1 [7, 8]\n"
  "conv3_1 [8, 9]\nconv3_b_0 [8, 8]\nconv3_w_0 [8, 8]\nconv3_2 [9, 10]\n"
  "conv4_1 [10, 11]\nconv4_b_0 [10, 10]\nconv4_w_0 [10, 10]\nconv4_2 [11, 12]\n"
  "conv5_w_0 [12, 12]\nconv5_b_0 [12, 12]\n"
  "conv5_1 [12, 13]\nconv5_2 [13, 14]\npool5_1 [14, 15]\n"
  "fc6_w_0 [15, 15]\nfc6_b_0 [15, 15]\n"
  "fc6_1 [15, 16]\nfc6_2 [16, 17]\n_fc6_mask_1 [17, 17]\nfc6_3 [17, 18]\n"
  "fc7_w_0 [18, 18]\nfc7_b_0 [18, 18]\n"
  "fc7_1 [18, 19]\nfc7_2 [19, 20]\nfc7_3 [20, 21]\n_fc7_mask_1 [20, 20]\n"
  "fc8_1 [21, 22]\nfc8_w_0 [21, 21]\nfc8_b_0 [21, 21]\n"
  "prob_1 [22, 22]\n";

SKYPAT_F(LivenessAnalysisTest, testAlexNet){
  GraphHelper graph(new xGraph());

  // add inputs.
  graph.addInput("data_0", {10, 3, 227, 227});
  graph.addInput("conv1_w_0", {96, 3, 11, 11});
  graph.addInput("conv1_b_0", {96});
  graph.addInput("conv2_w_0", {256, 48, 5, 5});
  graph.addInput("conv2_b_0", {256});
  graph.addInput("conv3_w_0", {384, 256, 3, 3});
  graph.addInput("conv3_b_0", {384});
  graph.addInput("conv4_w_0", {384, 192, 3, 3});
  graph.addInput("conv4_b_0", {384});
  graph.addInput("conv5_w_0", {256, 192, 3, 3});
  graph.addInput("conv5_b_0", {256});
  graph.addInput("fc6_w_0", {4096, 9216});
  graph.addInput("fc6_b_0", {4096});
  graph.addInput("fc7_w_0", {4096, 4096});
  graph.addInput("fc7_b_0", {4096});
  graph.addInput("fc8_w_0", {1000, 4096});
  graph.addInput("fc8_b_0", {1000});

  // add initializers.
  graph.addInitializer("conv1_w_0");
  graph.addInitializer("conv1_b_0");
  graph.addInitializer("conv2_w_0");
  graph.addInitializer("conv2_b_0");
  graph.addInitializer("conv3_w_0");
  graph.addInitializer("conv3_b_0");
  graph.addInitializer("conv4_w_0");
  graph.addInitializer("conv4_b_0");
  graph.addInitializer("conv5_w_0");
  graph.addInitializer("conv5_b_0");
  graph.addInitializer("fc6_w_0");
  graph.addInitializer("fc6_b_0");
  graph.addInitializer("fc7_w_0");
  graph.addInitializer("fc7_b_0");
  graph.addInitializer("fc8_w_0");
  graph.addInitializer("fc8_b_0");

  // create nodes (layers)
  graph.addNode("Conv",    {"data_0", "conv1_w_0", "conv1_b_0"})
       .addOutput("conv1_1", {1});
  graph.addNode("Relu",    {"conv1_1"}).addOutput("conv1_2", {1});
  graph.addNode("LRN",     {"conv1_2"}).addOutput("norm1_1", {1});
  graph.addNode("MaxPool", {"norm1_1"}).addOutput("pool1_1", {1});

  graph.addNode("Conv",    {"pool1_1", "conv2_w_0", "conv2_b_0"})
       .addOutput("conv2_1", {1});
  graph.addNode("Relu",    {"conv2_1"}).addOutput("conv2_2", {1});
  graph.addNode("LRN",     {"conv2_2"}).addOutput("norm2_1", {1});
  graph.addNode("MaxPool", {"norm2_1"}).addOutput("pool2_1", {1});

  graph.addNode("Conv",    {"pool2_1", "conv3_w_0", "conv3_b_0"})
       .addOutput("conv3_1", {1});
  graph.addNode("Relu",    {"conv3_1"}).addOutput("conv3_2", {1});

  graph.addNode("Conv",    {"conv3_2", "conv4_w_0", "conv4_b_0"})
       .addOutput("conv4_1", {1});
  graph.addNode("Relu",    {"conv4_1"}).addOutput("conv4_2", {1});

  graph.addNode("Conv",    {"conv4_2", "conv5_w_0", "conv5_b_0"})
       .addOutput("conv5_1", {1});
  graph.addNode("Relu",    {"conv5_1"}).addOutput("conv5_2", {1});
  graph.addNode("MaxPool", {"conv5_2"}).addOutput("pool5_1", {1});

  graph.addNode("Gemm",    {"pool5_1", "fc6_w_0", "fc6_b_0"})
       .addOutput("fc6_1", {1});
  graph.addNode("Relu",    {"fc6_1"}).addOutput("fc6_2", {1});
  graph.addNode("Dropout", {"fc6_2"}).addOutput("fc6_3", {1})
                                     .addOutput("_fc6_mask_1", {1});

  graph.addNode("Gemm",    {"fc6_3", "fc7_w_0", "fc7_b_0"})
       .addOutput("fc7_1", {1});
  graph.addNode("Relu",    {"fc7_1"}).addOutput("fc7_2", {1});
  graph.addNode("Dropout", {"fc7_2"}).addOutput("fc7_3", {1})
                                     .addOutput("_fc7_mask_1", {1});

  graph.addNode("Gemm",    {"fc7_3", "fc8_w_0", "fc8_b_0"})
       .addOutput("fc8_1", {1});
  graph.addNode("Softmax", {"fc8_1"}).addOutput("prob_1", {1});

  // set graph output value(s).
  graph.finish({"prob_1"});

  Module module(std::unique_ptr<xGraph>(graph.getGraph()));

  GraphLivenessAnalysis *liveAnalPass = CreateLivenessAnalysisPass();
  PassManager pm;
  pm.add(liveAnalPass);
  pm.run(module);

  std::string result;
  OStrStream oss(result);
  liveAnalPass->print(oss);

  ASSERT_TRUE(result == testAlexNetAnswer);
}
