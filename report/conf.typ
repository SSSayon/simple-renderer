#import "@preview/subpar:0.2.0"

#let conf(doc) = {

  set par(justify: true)

  set page(
    paper: "a4",
    numbering: "1",
  )

  set heading(numbering: "1.1. ")  
  show heading: set text(font: ("Libertinus Serif", "STHeiti"))

  // font
  set text(font: ("Libertinus Serif", "STSong"), lang: "zh", region: "cn")
  show emph: text.with(font: ("Libertinus Serif", "STKaiti"))
  show strong: text.with(font: ("Libertinus Serif", "STHeiti"))

  // code
  show raw.where(block: false): box.with(
    fill: luma(240),
    inset: (x: 3pt, y: 0pt),
    outset: (y: 3pt),
    radius: 2pt,
  )

  show raw.where(block: true): it => block( 
    // Layout
    width: 100%,
    fill: rgb("f2f2f2"),
    outset: (y: 3pt),
    radius: 1pt,
    stroke: (1.5pt + rgb("e6e6e6")),
  )[
    // Title's layout
    #block(
      width: 100%,
      fill: rgb("e6e6e6"),
      outset: (y: 5pt),
      radius: 0pt,
    )[#" " #str(it.lang)]

    // Line numbering
    #let num = it.lines.len()
    #let arr = (" ",)
    #for i in range(1, num+1).map(str) {
      if i.len() <= str(num).len() {
        i = (str(num).len()-i.len()+1) * " " + i
      }
      arr.push(i)
    }

    // Fill in elements
    #grid(
      columns: (1em + str(num).len() * 1em, auto),
      [
        #set text(
          fill: rgb("8e8a8a"),
        )
        #grid(
          rows: (0.45pt,) + num * (11.16pt,),
          ..arr
        )
      ],
      [
        #it
      ]
    )
  ]

  // link & reference
  show link: link => {
    set text(fill: rgb("#ee0000"))
    underline(link)
  }

  set ref(supplement: it => {""})
  show ref: it => {
      let el = it.element
      set text(fill: rgb("#ee0000"))
      if (el == none) [??] else { it }
  }

  // args
  doc
}

// font
#let bold_italic(txt) = {
  set text(font: ("Libertinus Serif", "STFangsong"))
  txt
}
#let italic(txt) = {
  set text(font: ("Libertinus Serif", "STKaiti"))
  txt
}
#let bold(txt) = {
  set text(font: ("Libertinus Serif", "STHeiti"))
  txt
}

// title & author
#let title(txt) = {
  align(strong(text(txt, size: 20pt)), center)
}
#let author(txt) = {
  align(italic(text(txt, size: 12pt)), center)
}

// table
#import "@preview/tablex:0.0.6": tablex, hlinex
#import "@preview/tablem:0.1.0": tablem
#let three-line-table-l = tablem.with(
  render: (..args) => {
    tablex(
      columns: auto,
      auto-lines: false,
      align: left + horizon,
      hlinex(y: 0),
      hlinex(y: 1, stroke: 0.5pt),
      ..args,
      hlinex(),
    )
  }
)
#let table3-l(cap, args) = figure(
  three-line-table-l(args),
  supplement: [表],
  kind: table,
  caption: {
    if cap == [] { none } else { cap }
  }
)

#let three-line-table-c = tablem.with(
  render: (..args) => {
    tablex(
      columns: auto,
      auto-lines: false,
      align: center + horizon,
      hlinex(y: 0),
      hlinex(y: 1, stroke: 0.5pt),
      ..args,
      hlinex(),
    )
  }
)
#let table3-c(cap, args) = figure(
  three-line-table-c(args),
  supplement: [表],
  kind: table,
  caption: {
    if cap == [] { none } else { cap }
  }
)
