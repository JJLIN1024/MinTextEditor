# Min Text Editor

This text editor is inspired by [kilo-tutorial](https://github.com/snaptoken/kilo-tutorial), this project follows Chromium coding styles using [clang-format](https://clang.llvm.org/docs/ClangFormatStyleOptions.html).

## Support OS

- Linux(Ubuntu)
- MacOS

## Usage

1. clone this repository into your desired location, e.g. `dir`.
2. `cd dir`
3. `make`
4. `./min <file to open>`, `<file to open>` is optional.

## Support Keys

### Non-Vim Keys
- `<Ctrl> + s`: save file
- `<Ctrl> + q`: quit

### Normal Mode
- `i`: enter **Insert Mode**
- `x`: delete character where the cursor currently at
- `gg`: scroll to the top
- `G`: scroll to the buttom
- `zz`: center the cursor
- `0`/`HOME`: move cursor to the start of the line
- `$`/`END`: move cursor to the end of the line
- `←`/`→`/`↑`/`↓`: move cursor to the left/right/up/down
- `h`/`l`/`k`/`j`: move cursor to the left/right/up/down
- `/<search pattern>`: search
- `:<command>`: below are supported commands
    - `w`: save file
    - `q`: quit
    - `wq`: save file and then quit
    - `q!`: force quit
### Insert Mode

Insert Mode works just like normal text editor, simply insert text & delete text, use arrow keys to move around.

- `<Esc>`/`jk`/`jj`: enter **Normal Mode**


## Features & Future Goals

- [x] adjust window size dynamically
- [ ] ./vimrc customizations
- [ ] visual mode/ copy and paste
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

