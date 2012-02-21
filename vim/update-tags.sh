#!/bin/sh
find \
    -name "*.h" -o -name "*.inl" -o -name "*.cpp"\
    | grep -v '/examples\?/' \
    | grep -v '/samples\?/' \
    | grep -v '/libs/.*/test/' \
    > cscope.files

cscope -b
ctags --c++-kinds=+p --fields=+iaS --extra=+q --language-force=c++ --excmd=number -Lcscope.files
