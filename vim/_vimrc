" {{{
" Descirption: vimfile for Linux/windows
" Last Update: 2013-01-26
" Author: kenwell - kenwell.ustc AT Gmail DOT com
" }}}

let mapleader = ";"    " 比较习惯用;作为命令前缀，右手小拇指直接能按到

" 判断操作系统
if (has("win32") || has("win64") || has("win32unix"))
    let g:isWin = 1
else
    let g:isWin = 0
endif

" 判断是终端还是gvim
if has("gui_running")
    let g:isGUI = 1
else
    let g:isGUI = 0
endif

" 快捷打开编辑vimrc文件的键盘绑定
map <silent> <leader>ee :e $HOME/.vimrc<cr>
autocmd! bufwritepost *.vimrc source $HOME/.vimrc

" 保存文件
nmap <leader>ww :w!<cr>
" ^z快速进入shell
nmap <C-Z> :shell<cr>

set nobackup        " 关闭备份
set noswapfile      " 不使用swp文件，注意，错误退出后无法恢复
set lbr             " 在breakat字符处而不是最后一个字符处断行

set nocompatible    " 关闭vi兼容模式
set number          " 显示行号
set showcmd         " 显示命令
set lz              " 当运行宏时，在命令执行完成之前，不重绘屏幕
set hid             " 可以在没有保存的情况下切换buffer

syntax enable       " 语法高亮
syntax on           " 关键字上色
filetype plugin on  " 文件类型插件
filetype indent on  " 文件语法自动缩进
autocmd BufEnter * :syntax sync fromstart "Highlight from start of file

set autoindent      " 自动缩进
set smartindent     " 智能缩进
set cindent         " C/C++风格缩进
set autoindent      " 自动缩进,和上一行一样缩进

" command line completion operates show match above command line  
set wildmenu        

" tab转化为4个字符
set expandtab
set smarttab
set shiftwidth=4    " indent space
set tabstop=4       " tab space character

set backspace=eol,start,indent " 设置随意删除,删除到上一行
set whichwrap+=<,>,h,l " 退格键和方向键可以换行

set incsearch       " 在输入要搜索的文字时，vim会实时匹配
set hlsearch        " 高亮搜索
set ignorecase      " 搜索时忽略大小写
set magic           " 额，自己:h magic吧，一行很难解释
set showmatch       " 显示匹配的括号,类似当输入一个左括号时会匹配相应的那个右括号

" 不使用beep或flash
set vb t_vb=
set history=400  " vim记住的历史操作的数量，默认的是20
set autoread     " 当文件在外部被修改时，自动重新读取
set mouse=a      " 在所有模式下都允许使用鼠标，还可以是n,v,i,c等

"colorscheme desert
 
" {{{ 开始折叠
set foldenable
" 设置语法折叠
" manual  手工定义折叠
" indent  更多的缩进表示更高级别的折叠
" expr    用表达式来定义折叠
" syntax  用语法高亮来定义折叠
" diff    对没有更改的文本进行折叠
" marker  对文中的标志折叠
set foldmethod=marker
"折叠相关的快捷键
"zR 打开所有的折叠
"za Open/Close (toggle) a folded group of lines.
"zA Open a Closed fold or close and open fold recursively.
"zi 全部 展开/关闭 折叠
"zo 打开 (open) 在光标下的折叠
"zc 关闭 (close) 在光标下的折叠
"zC 循环关闭 (Close) 在光标下的所有折叠
"zM 关闭所有可折叠区域
" 设置折叠区域的宽度
set foldcolumn=0
" 设置折叠层数为
setlocal foldlevel=1
" 新建的文件，刚打开的文件不折叠
autocmd! BufNewFile,BufRead * setlocal nofoldenable
" }}}

"在gvim中高亮当前行
if (g:isGUI)
    set cursorline
    hi cursorline guibg=#333333
    hi CursorColumn guibg=#333333
    set guifont=Bitstream_Vera_Sans_Mono:h10:cANSI
    set guifontwide=幼圆:h10.5:cGB2312
endif

" 设置字符集编码，默认使用utf8
if (g:isWin)
    let &termencoding=&encoding " 通常win下的encoding为cp936
    set fileencodings=utf8,cp936,ucs-bom,latin1
else
    set encoding=utf8
    set fileencodings=utf8,gb2312,gb18030,ucs-bom,latin1
endif

" 状态栏
set laststatus=2      " 总是显示状态栏
highlight StatusLine cterm=bold ctermfg=yellow ctermbg=blue
" 获取当前路径，将$HOME转化为~
function! CurDir()
    let curdir = substitute(getcwd(), $HOME, "~", "g")
    return curdir
endfunction
set statusline=[%n]\ %f%m%r%h\ \|\ \ pwd:\ %{CurDir()}\ \ \|%=\|\ %l,%c\ %p%%\ \|\ ascii=%b,hex=%b%{((&fenc==\"\")?\"\":\"\ \|\ \".&fenc)}\ \|\ %{$USER}\ @\ %{hostname()}\

" 根据给定方向搜索当前光标下的单词，结合下面两个绑定使用
function! VisualSearch(direction) range
    let l:saved_reg = @"
    execute "normal! vgvy"
    let l:pattern = escape(@", '\\/.*$^~[]')
    let l:pattern = substitute(l:pattern, "\n$", "", "")
    if a:direction == 'b'
        execute "normal ?" . l:pattern . "<cr>"
    else
        execute "normal /" . l:pattern . "<cr>"
    endif
    let @/ = l:pattern
    let @" = l:saved_reg
endfunction
" 用 */# 向 前/后 搜索光标下的单词
vnoremap <silent> * :call VisualSearch('f')<CR>
vnoremap <silent> # :call VisualSearch('b')<CR>

" 用c-j,k在buffer之间切换
nn <C-J> :bn<cr>
nn <C-K> :bp<cr>

" 恢复上次文件打开位置
set viminfo='10,\"100,:20,%,n~/.viminfo
au BufReadPost * if line("'\"") > 0|if line("'\"") <= line("$")|exe("norm '\"")|else|exe "norm $"|endif|endif

" 删除buffer时不关闭窗口
command! Bclose call <SID>BufcloseCloseIt()
function! <SID>BufcloseCloseIt()
    let l:currentBufNum = bufnr("%")
    let l:alternateBufNum = bufnr("#")

    if buflisted(l:alternateBufNum)
        buffer #
    else
        bnext
    endif

    if bufnr("%") == l:currentBufNum
        new
    endif

    if buflisted(l:currentBufNum)
        execute("bdelete! ".l:currentBufNum)
    endif
endfunction

" 缩写
iab idate <c-r>=strftime("%Y-%m-%d")<CR>
iab itime <c-r>=strftime("%H:%M")<CR>
iab imail wuguilin@qiyi.com
iab igmail kenwell.ustc@@gmail.com
iab iname Guilin Wu
iab iauth Author: Guilin Wu (wuguilin@qiyi.com)

" 插件窗口的宽度，如TagList,NERD_tree等，自己设置
let s:PlugWinSize = 30

" Coding "{{{
if has("win32")
    au FileType c,cpp setlocal tags+=$VIM/tags/stl.tags,$VIM/tags/boost.tags
else
    au FileType c,cpp setlocal tags+=$HOME/.tags/stl.tags,$HOME/.tags/boost.tags
endif

set tags+=./tags;$HOME " walk directory tree upto $HOME looking for tags
set completeopt=menuone,menu,longest,preview  " 智能补全

" Highlight space errors in C/C++ source files (Vim tip #935)
let c_space_errors=1
let java_space_errors=1

"设置C++搜索路径
if has("win32")
    set path=.,e:\Program\\\ Files\Microsoft\\\ Visual\\\ Studio\\\ .NET\\\ 2003\Vc7\include,e:\Program\\\ Files\Microsoft\\\ Visual\\\ Studio\\\ .NET\\\ 2003\Vc7\atlmfc\include,e:\libs\boost_1_43_0\boost,,
endif
" "}}}

" Coding Helper Functions "{{{
function! RemoveTrailingSpace()
    if $VIM_HATE_SPACE_ERRORS != '0' &&
          \(&filetype == 'c' || &filetype == 'cpp' || &filetype == 'vim')
        normal m`
        silent! :%s/\s\+$//e
        normal ``
    endif
endfunction

function! OpenCProject()
    if has("cscope")
        let db = findfile("cscope.out", ";")
        if (!empty(db))
            let path = strpart(db, 0, match(db, "/cscope.out$"))
            set nocscopeverbose " suppress 'duplicate connection' error
            exe "cs add " . db . " " . path
            set cscopeverbose
        endif
    endif

    if     findfile("Jamfile", ";") != "" ||
          \findfile("Jamroot", ";") != "" ||
          \findfile("Jamfile.v2", ";") != "" ||
          \findfile("Jamroot.v2", ";") != ""
      set makeprg=bjam
    endif
endfunction

" Command and Auto commands " {{{
" Sudo write
" comm! W exec 'w !sudo tee % > /dev/null' | e!

"Auto commands

if has("win32")
  au GUIEnter * simalt ~x " 启动时自动全屏
endif

au BufReadPost * if line("'\"") > 0 && line("'\"") <= line("$") | execute "normal g'\"" | endif " restore position in file

au! BufNewFile * set fenc=utf8 ff=unix

" automatically open and close the popup menu / preview window
au CursorMovedI,InsertLeave * if pumvisible() == 0|silent! pclose|endif

" " }}}

" Filetype detection " {{{
au BufRead,BufNewFile {Gemfile,Rakefile,Capfile,*.rake,config.ru} setf ruby
au BufRead,BufNewFile {*.md,*.mkd,*.markdown} setf markdown
au BufRead,BufNewFile {COMMIT_EDITMSG}  setf gitcommit

" Recognize standard C++ headers
au BufEnter /usr/include/c++/*          setf cpp
au BufEnter /usr/include/g++-3/*        setf cpp

au BufRead,BufNewFile todo.txt,done.txt setf todo
au BufRead,BufNewFile *.mm              setf xml
au BufRead,BufNewFile *.proto           setf proto
au BufRead,BufNewFile Jamfile*,Jamroot* setf jam
au BufRead,BufNewFile pending.data,completed.data setf task
" "}}}

" Filetype related autosettings " {{{
au FileType jam   set makeprg=bjam
au FileType diff  setlocal shiftwidth=4 tabstop=4
au FileType html  setlocal autoindent indentexpr=
au FileType changelog setlocal textwidth=76
" 把-等符号也作为xml文件的有效关键字，可以用Ctrl-N补全带-等字符的属性名
au FileType {xml,xslt} setlocal iskeyword=@,-,\:,48-57,_,128-167,224-235
au FileType xml        exe 'setlocal equalprg=xmllint\ --format\ --recover\ -'
" "}}}

" 根据不同的文件类型设定<F3>时应该查找的文件 "{{{
au FileType *             let b:vimgrep_files=expand("%:e") == "" ? "**/*" : "**/*." . expand("%:e")
au FileType c,cpp         let b:vimgrep_files="**/*.cpp **/*.cxx **/*.c **/*.h **/*.hpp"
au FileType php           let b:vimgrep_files="**/*.php **/*.htm **/*.html"
au FileType cs            let b:vimgrep_files="**/*.cs"
au FileType vim           let b:vimgrep_files="**/*.vim"
au FileType javascript    let b:vimgrep_files="**/*.js **/*.htm **/*.html"
au FileType python        let b:vimgrep_files="**/*.py"
au FileType xml           let b:vimgrep_files="**/*.xml"
" "}}}

" C++ code auto commands "{{{
au FileType c,cpp inoremap  <buffer>  /**<CR>     /**<CR><CR>/<Esc>kA<Space>@brief<Space>
au FileType c,cpp inoremap  <buffer>  /**<        /**<<Space>@brief<Space><Space>*/<Left><Left><Left>
au FileType c,cpp vnoremap  <buffer>  /**<        /**<<Space>@brief<Space><Space>*/<Left><Left><Left>
au FileType c,cpp inoremap  <buffer>  /**<Space>  /**<Space>@brief<Space><Space>*/<Left><Left><Left>
au FileType c,cpp vnoremap  <buffer>  /**<Space>  /**<Space>@brief<Space><Space>*/<Left><Left><Left>

au FileType c,cpp setlocal include=^\\s*#\\s*include\\s*\"\\zs[^\"]*\\ze\"

" Detect if the current file type is a C-like language.
" au BufNewFile,BufRead *.c,*.cpp,*.objc,*.mm call SetupForCLang()

" Setting for files following the GNU coding standard
au BufEnter /usr/*                  setlocal
                                        \ cinoptions=>4,n-2,{2,^-2,:2,=2,g0,h2,p5,t0,+2,(0,u0,w1,m1
                                        \ shiftwidth=2
                                        \ tabstop=8

"" Remove trailing spaces for C/C++ and Vim files
"au BufWritePre *                  call RemoveTrailingSpace()

" "}}}

" asciidoc autocommands "{{{
"au BufRead,BufNewFile *.txt   set filetype=asciidoc
au FileType asciidoc      setlocal shiftwidth=2
                               \ tabstop=2
                               \ textwidth=70 wrap formatoptions=tcqnmB
                               \ makeprg=asciidoc\ -a\ numbered\ -a\ toc\ %
                               \ errorformat=ERROR:\ %f:\ line\ %l:\ %m
                               \ foldexpr=MyAsciidocFoldLevel(v:lnum)
                               \ foldmethod=expr
                               \ formatlistpat=^\\s*\\d\\+\\.\\s\\+\\\\|^\\s*<\\d\\+>\\s\\+\\\\|^\\s*[a-zA-Z.]\\.\\s\\+\\\\|^\\s*[ivxIVX]\\+\\.\\s\\+
                               \ comments=s1:/*,ex:*/,://,b:#,:%,:XCOMM,fb:-,fb:*,fb:+,fb:.,fb:>
" "}}}

" python autocommands "{{{

" Python 自动补全功能，用 Ctrl-N 调用
" isk+=.,表示把小数点和逗号作为word的一部分
" 需要把从http://www.vim.org/scripts/script.php?script_id=850下载的
" pydiction文件放到~/.vim下
"au FileType python set complete+=k~/.vim/tools/pydiction "isk+=.,
let g:pydiction_location = '~/.vim/tools/pydiction/complete-dict'
"defalut g:pydiction_menu_height == 15
""let g:pydiction_menu_height = 20 

" 设定python的makeprg
au FileType python set makeprg=python\ -c\ \"import\ py_compile,sys;\ sys.stderr=sys.stdout;\ py_compile.compile(r'%')\"
"au FileType python set efm=%C\ %.%#,%A\ \ File\ \"%f\"\\\,\ line\ %l%.%#,%Z%[%^\ ]%\\@=%m
au FileType python set efm=%[%^(]%\\+('%m'\\,\ ('%f'\\,\ %l\\,\ %v\\,%.%#
au FileType python set smartindent cinwords=if,elif,else,for,while,try,except,finally,def,class
" "}}}

" Plugins " {{{

" config bundle and pathogen
let s:disabled_bundles = []

" Bundle: pathogen.vim
" http://www.vim.org/scripts/script.php?script_id=2332
runtime bundle/pathogen.vim/plugin/pathogen.vim
let g:pathogen_disabled = s:disabled_bundles
call pathogen#infect() 

" Bundle: AutoTag
" http://www.vim.org/scripts/script.php?script_id=1343

" Indents " {{{
" Bundle: indentpython.vim--nianyang
" " }}}

" Bundle: snipMate
if finddir("snippets-custom", &runtimepath) != ""
    let g:snippets_dir = fnamemodify(finddir("snippets-custom", &runtimepath), ":p")
endif
let g:snips_author = "wugl"   "wugl 

" Bundle: taglist
" http://www.vim.org/scripts/script.php?script_id=273
" <leader>t 打开TagList窗口，窗口在右边
nmap <silent> <leader>t :TlistToggle<cr>
"let Tlist_Ctags_Cmd = '/usr/bin/ctags'
let Tlist_Show_One_File = 0
let Tlist_Exit_OnlyWindow = 1 
let Tlist_Use_Right_Window = 1
let Tlist_File_Fold_Auto_Close = 1
let Tlist_GainFocus_On_ToggleOpen = 0
let Tlist_WinWidth = s:PlugWinSize
let Tlist_Auto_Open = 0
let Tlist_Display_Prototype = 0
"let Tlist_Close_On_Select = 1

" Bundle: OnnicppComplete
" http://www.vim.org/scripts/script.php?script_id=1520
set completeopt=menu
let OmniCpp_ShowPrototypeInAbbr = 1 
let OmniCpp_DefaultNamespaces = ["std"]     " 逗号分割的字符串
let OmniCpp_MayCompleteScope = 1 
let OmniCpp_ShowPrototypeInAbbr = 0 
let OmniCpp_SelectFirstItem = 2 
" c-j自动补全，当补全菜单打开时，c-j,k上下选择
imap <expr> <c-j>      pumvisible()?"\<C-N>":"\<C-X><C-O>"
imap <expr> <c-k>      pumvisible()?"\<C-P>":"\<esc>"
" f:文件名补全，l:行补全，d:字典补全，]:tag补全
imap <C-]>             <C-X><C-]>
imap <C-F>             <C-X><C-F>
imap <C-D>             <C-X><C-D>
imap <C-L>             <C-X><C-L> 

" Bundle: NERD_commenter
" http://www.vim.org/scripts/script.php?script_id=1218
" Toggle单行注释/“性感”注释/注释到行尾/取消注释
map <leader>cc ,c<space>
map <leader>cs ,cs
map <leader>c$ ,c$
map <leader>cu ,cu

" Bundle: NERD tree
" http://www.vim.org/scripts/script.php?script_id=1658
let NERDTreeShowHidden = 1
let NERDTreeWinPos = "right"
let NERDTreeWinSize = s:PlugWinSize 
nmap <leader>n :NERDTreeToggle<cr>

" Bundle: DoxygenToolkit.vim
" http://www.vim.org/scripts/script.php?script_id=987
" 暂时没有使用

" Bundle: ctags and scope
" 更新ctags和cscope索引
" href: http://www.vimer.cn /2009/10/把vim打造成一个真正的ide2.html
" 稍作修改，提取出DeleteFile函数，修改ctags和cscope执行命令
map <F6> :call Do_CsTag()<cr>
function! Do_CsTag()
    let dir = getcwd()

    "先删除已有的tags和cscope文件，如果存在且无法删除，则报错。
    if ( DeleteFile(dir, "tags") ) 
        return 
    endif
    if ( DeleteFile(dir, "cscope.files") ) 
        return 
    endif
    if ( DeleteFile(dir, "cscope.out") ) 
        return 
    endif

    if(executable('ctags'))
        silent! execute "!ctags -R --c++-kinds=+p --fields=+iaS --extra=+q ."
    endif
    if(executable('cscope') && has("cscope") )
        if(g:isWin)
            silent! execute "!dir /s/b *.c,*.cpp,*.h,*.java,*.cs >> cscope.files"
        else
            silent! execute "!find . -iname '*.[ch]' -o -name '*.cpp' > cscope.files"
        endif
        silent! execute "!cscope -b"
        execute "normal :"
        if filereadable("cscope.out")
            execute "cs add cscope.out"
        endif
    endif
    " 刷新屏幕
    execute "redr!"
endfunction

function! DeleteFile(dir, filename)
    if filereadable(a:filename)
        if (g:isWin)
            let ret = delete(a:dir."\\".a:filename)
        else
            let ret = delete("./".a:filename)
        endif
        if (ret != 0)
            echohl WarningMsg | echo "Failed to delete ".a:filename | echohl None
            return 1
        else
            return 0
        endif
    endif
    return 0
endfunction

" cscope 绑定
if has("cscope")
    set csto=1
    set cst
    set nocsverb
    if filereadable("cscope.out")
        cs add cscope.out
    endif
    set csverb
    " s: C语言符号  g: 定义     d: 这个函数调用的函数 c: 调用这个函数的函数
    " t: 文本       e: egrep模式    f: 文件     i: include本文件的文件
    nmap <leader>ss :cs find s <C-R>=expand("<cword>")<CR><CR>
    nmap <leader>sg :cs find g <C-R>=expand("<cword>")<CR><CR>
    nmap <leader>sc :cs find c <C-R>=expand("<cword>")<CR><CR>
    nmap <leader>st :cs find t <C-R>=expand("<cword>")<CR><CR>
    nmap <leader>se :cs find e <C-R>=expand("<cword>")<CR><CR>
    nmap <leader>sf :cs find f <C-R>=expand("<cfile>")<CR><CR>
    nmap <leader>si :cs find i ^<C-R>=expand("<cfile>")<CR>$<CR>
    nmap <leader>sd :cs find d <C-R>=expand("<cword>")<CR><CR>
endif

" Bundle: QuickFix
" Quick Fix 设置
map <leader>cw :cw<cr>
map <F3> :cp<cr>
map <F4> :cn<cr>
set cscopequickfix=s-,c-,d-,i-,t-,e-

" Bundle: Lookup file
" lookup file
" http://www.vim.org/scripts/script.php?script_id=1581
" 增加一行（line 295）: let pattern = '\c' . a:pattern
" 按F5或者;ff打开文件搜索窗口
let g:LookupFile_MinPatLength = 0
let g:LookupFile_PreserveLastPattern = 0
let g:LookupFile_PreservePatternHistory = 0
let g:LookupFile_AlwaysAcceptFirst = 1
let g:LookupFile_AllowNewFiles = 0
if filereadable("./filenametags")
  let g:LookupFile_TagExpr = '"./filenametags"'
endif
nmap <silent> <leader>ff :LookupFile<cr>

function! LookupFile_IgnoreCaseFunc(pattern)
    let _tags = &tags
    try
        let &tags = eval(g:LookupFile_TagExpr)
        let newpattern = '\c' . a:pattern
        let tags = taglist(newpattern)
    catch
        echohl ErrorMsg | echo "Exception: " . v:exception | echohl NONE
        return ""
    finally
        let &tags = _tags
    endtry

    " Show the matches for what is typed so far.
    let files = map(tags, 'v:val["filename"]')
    return files
endfunction
let g:LookupFile_LookupFunc = 'LookupFile_IgnoreCaseFunc' 

" Bundle: Buffers Explorer
" Buffers Explorer （需要genutils.vim）
" http://vim.sourceforge.net/scripts/script.php?script_id=42
" http://www.vim.org/scripts/script.php?script_id=197
let g:bufExplorerDefaultHelp=0       " Do not show default help.
let g:bufExplorerShowRelativePath=1  " Show relative paths.
let g:bufExplorerSortBy='mru'        " Sort by most recently used.
let g:bufExplorerSplitRight=0        " Split left.
let g:bufExplorerSplitVertical=1     " Split vertically.
let g:bufExplorerSplitVertSize = s:PlugWinSize  " Split width
let g:bufExplorerUseCurrentWindow=1  " Open in new window.
autocmd BufWinEnter \[Buf\ List\] setl nonumber
nmap <silent> <Leader>b :BufExplorer<CR>

" Bundle: Winmanager
""""""""""""""""""""""""""""""
" winManager setting
" http://www.vim.org/scripts/script.php?script_id=95
""""""""""""""""""""""""""""""
let g:winManagerWindowLayout = "BufExplorer,FileExplorer|TagList"
let g:winManagerWidth = 30
let g:defaultExplorer = 0
nmap <C-W><C-F> :FirstExplorerWindow<cr>
nmap <C-W><C-B> :BottomExplorerWindow<cr>
nmap <silent> <leader>wm :WMToggle<cr> 

" Bundle: FSwitch
" http://www.vim.org/scripts/script.php?script_id=2590
cabbrev A FSHere " 可以用:A在.h/.cpp间切换
au! BufEnter *.h let b:fswitchlocs='reg:/include/src/,reg:/include.*/src/,ifrel:|/include/|../src|,reg:!sscc\(/[^/]\+\|\)/.*!libs\1/**!'
au! BufEnter *.c,*.cpp let b:fswitchlocs='reg:/src/include/,reg:|src|include/**|,ifrel:|/src/|../include|,reg:|libs/.*|**|'


" Bundle: DrawIt
" http://www.vim.org/scripts/script.php?script_id=40

" Syntaxes " {{{
" Bundle: asciidoc.vim
" " }}}

" Bundle: altercation/vim-colors-solarized
set background=dark
let g:solarized_visibility = "low"
colorscheme solarized

" " }}}
