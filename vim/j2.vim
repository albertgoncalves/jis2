" $ cp j2.vim ~/.vim/syntax/j2.vim
" $ grep '.j2' ~/.vimrc
" autocmd BufNewFile,BufRead *.j2 setlocal filetype=j2

if exists("b:current_syntax")
    finish
endif

syn match Comment   ";.*$"

let b:current_syntax = "j2"
