#let slide(title, bullets, image-data) = {
  set page(width: 297mm, height: 210mm, margin: 1cm)
  set text(font: "Noto Sans", size: 20pt, fill: rgb("243447"))

  align(center + horizon)[
    #grid(
      columns: (1fr, 0.92fr),
      gutter: 0.25in,
      align(left + horizon)[
        #text(size: 24pt, weight: "bold", fill: rgb("12355b"))[#title]
        #v(0.15in)
        #for bullet in bullets {
          block(width: 100%)[
            #text(fill: rgb("e07a5f"))[•]
            #h(0.08in)
            #bullet
            #v(0.08in)
          ]
        }
        
      ],
      align(center + horizon)[
        #image(image-data, width: 100%, fit: "contain")
      ],
    )
    #text(fill: rgb("777777"))[Milan Pickavet 261360695 ECSE 541]
  ]
  
}
