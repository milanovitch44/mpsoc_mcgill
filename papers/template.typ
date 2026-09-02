#let slide(title, bullets, image-data) = {
  set page(width: 13.333in, height: 7.5in, margin: 0.55in)
  set text(font: "Aptos", size: 20pt, fill: rgb("243447"))

  align(center + horizon)[
    grid(
      columns: (1fr, 0.82fr),
      gutter: 0.55in,
      align(left + center)[
        [
          text(size: 31pt, weight: "bold", fill: rgb("12355b"))[#title]
          v(0.35in)
          for bullet in bullets {
            block(width: 100%)[
              grid(columns: (0.18in, 1fr), gutter: 0.12in)[
                text(fill: rgb("e07a5f"))[•]
                bullet
              ]
              v(0.16in)
            ]
          }
        ]
      ],
      align(center + center)[
        image.decode(image-data, width: 100%, height: 4.6in, fit: "contain")
      ],
    )
  ]
}
