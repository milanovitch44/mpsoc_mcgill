#import "template.typ": slide

#let wolf_image = """
<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 640 480">
  <rect width="640" height="480" rx="24" fill="#e8f1f2"/>
  <circle cx="500" cy="115" r="58" fill="#f2cc8f"/>
  <path d="M92 365 C130 220 240 160 360 190 C450 212 510 285 548 365 Z" fill="#12355b"/>
  <path d="M165 220 L145 80 L245 165 Z M345 180 L410 70 L425 225 Z" fill="#12355b"/>
  <path d="M222 270 C270 228 350 230 398 270 C370 330 245 330 222 270 Z" fill="#f4f1de"/>
  <circle cx="270" cy="250" r="13" fill="#243447"/>
  <circle cx="355" cy="250" r="13" fill="#243447"/>
  <path d="M292 303 Q320 322 348 303" fill="none" stroke="#e07a5f" stroke-width="10" stroke-linecap="round"/>
</svg>
""".encode()

#slide(
  [Wolf08: An Early Multicore Proposal],
  (
    [A compact architecture designed for throughput-oriented workloads.],
    [Multiple simple cores share a fast on-chip interconnect.],
    [The paper highlights regularity, scalability, and energy efficiency.],
  ),
  wolf_image,
)
