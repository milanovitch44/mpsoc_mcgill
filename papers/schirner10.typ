#import "template.typ": slide

#let slide_image = "schirner_trade_off.png"
#slide(
  [Fast and Accurate Processor Models
    for Efficient MPSoC Design],
  (
    [Different levels of complexity: speed/accuracy trade-off in system simulation],
    [Instruction Set Simulator (ISS, traditional)],
    [Bus-Functional Modeling (BFM)],
    [Transaction-Level Modeling (TLM) (interrupts, scheduling)],
    [*Question*: How can FPGA simulations predict the performance of ASICs?],
  ),
  slide_image,
)
// Previous: Instruction Set Simulator, now unfeasible, so we use models
// Very useful for design space exploration
// There is a clear speed/accuracy trade-off in system simulation, and this model strikes a good balance
// TLM: Transaction-Level Modeling
// BFM: Bus-functional Modeling
