#import "template.typ": slide

#let wolf_image = "wolf08_stnomadik.png"
#slide(
  [Wolf08: Multiprocessor SoC Technology],
  (
    [History: Why? real-time, low-power & multitasking],
    [Video, audio, (irregular) memory, I/O, multicore],
    [CAD: Platform-based design, interconnects],
    [*Question*: Does the CPU often contain instructions that also accelerate video/audio acceleration? ]
  ),
  wolf_image,
)
