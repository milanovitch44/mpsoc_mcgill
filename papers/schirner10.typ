#import "template.typ": slide

#let wolf_image = "schirner_trade_off.png"
#slide(
  [Fast and Accurate Processor Models
    for Efﬁcient MPSoC Design],
  (
    [Different levels of complexity: speed/accuracy trade-off in system simulation],
    [Transaction-level modeling],
    [Bus-functional modeling],
    [Instruction set simulator],
    [*Question*: How can FPGA simulations predict the performance of ASIC's?],
  ),
  wolf_image,
)
// Previous: Instruction Set Simulator, now unfeasible, so we use models
// Very useful for design space exploration
// There is a clear speed/accuracy trade-off in system simulation, and this model strikes a good balance
// TLM: Transaction-Level Modelling
// BFM: Bus-functional Modeling
