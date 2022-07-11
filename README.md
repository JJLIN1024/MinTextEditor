# KiloTextEditor

This text editor is inspired by [kilo-tutorial](https://github.com/snaptoken/kilo-tutorial), this project follows Chromium coding styles using [clang-format](https://clang.llvm.org/docs/ClangFormatStyleOptions.html).

## Usage

1. clone this repository into your desired location, e.g. `dir`.
2. `cd dir`
3. `make`
4. `./min <file to open>`, `<file to open>` is optional.

## Support Keys

### Non-Vim Keys


### Normal Mode

- i: enter **Insert Mode**
- gg: scroll to the top
- G: scroll to the buttom
- 0/HOME: move cursor to the start of the line
- $/END: move cursor to the end of the line
- &#8592: move cursor to the right


### Insert Mode

## Features & Future Goals

- [x] adjust window size dynamically
- [ ] cursor movement support: UP & DOWN alignment when row involves TAB
- [ ] add unit test
- [ ] syntax highlight support for C, python, cpp
- [ ] add CI/CD, test support for Linux & Windows
- [ ] vim-like functionality(mainly navigation)
- [ ] hightlight current cursor row
- [ ] wide character support（中文）
- [ ] hybrid data structure(array/rope/gap buffer)
- [ ] file tree
- [ ] fuzzy search
- [ ] regex replace(all)
- [ ] split view
- [ ] Mouse support

