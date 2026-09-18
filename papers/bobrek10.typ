#import "template.typ": slide

#let slide_image = "bobrek_image.png"
#slide(
  [Stochastic Contention Level Simulation for
    Single-Chip Heterogeneous Multiprocessors],
  (
    [Contention: competition over shared resources (memory, buses)],
    [Clock-Accurate: Slow, but accurate],
    [SCL: use CA simulation to extract attributes],
    [Run 1M instructions stochastically],
    [*Question*: Can CA simulation results be reused?],
  ),
  slide_image,
)