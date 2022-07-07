# KiloTextEditor

This text editor is inspired by [kilo-tutorial](https://github.com/snaptoken/kilo-tutorial), this project follows Chromium coding styles using [clang-format](https://clang.llvm.org/docs/ClangFormatStyleOptions.html).

## Usage

1. clone this repository into your desired location, e.g. `dir`.
2. `cd dir`
3. `make`
4. `./min <file to open>`, `<file to open>` is optional.

## Features

- [x] Adjust window size dynamically.
- [ ] Mouse support

## Bug

- [ ] wrong line number displayed at the end of the file, say the number of lines in the displayed file is $n$, then when scrolling to the end of the file, the status bar will display $n+1/n$, which should be $n/n$.

- [ ] some line will be missing when resizing editor window via mouse. resizing is implemented through signal listening(SIGWINCH).
