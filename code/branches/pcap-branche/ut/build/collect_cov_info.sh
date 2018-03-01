#!/bin/bash

# directories
DIR_PREFIX=`pwd`/../../src
ALL_DIRS="$DIR_PREFIX/pub \
          $DIR_PREFIX/frame \
          $DIR_PREFIX/protocol \
          $DIR_PREFIX/protocol/l2 \
          $DIR_PREFIX/protocol/l3 \
          $DIR_PREFIX/protocol/l4 \
          $DIR_PREFIX/protocol/l4/tcp \
          $DIR_PREFIX/protocol/l4/udp \
          $DIR_PREFIX/protocol/l7 \
          $DIR_PREFIX/protocol/l7/http"

# create gcov temperary directory
gcov_dir="`pwd`/gcov"
[ -e $gcov_dir ] && rm -fr $gcov_dir
mkdir $gcov_dir

# copy all .gcno and .gcda files
for dir in $ALL_DIRS; do
    cd $dir
    cpp_files=`ls | grep '\.cpp$'`
    for cpp_file in $cpp_files; do
        gcno_file="`echo $cpp_file | awk -F. '{print $1}'`.gcno"
        cp -f "`pwd`/$gcno_file" $gcov_dir
        
        gcda_file="`echo $cpp_file | awk -F. '{print $1}'`.gcda"
        cp -f "`pwd`/$gcda_file" $gcov_dir
    done
    cd -
done

# generate text format coverage info files
cd $gcov_dir
lcov -c -d ./ -o tmas_ut_cov.info

# generate html formate coverage info files
genhtml tmas_ut_cov.info -o tmas_ut_cov

# fail then leave with tmp dirs and files for debug
[ $? -ne 0 ] && exit 1

# copy html files to httpd doc root
cp -fr tmas_ut_cov /var/www/html/

# delete gcov directory
cd -
rm -fr $gcov_dir

# clear coverage files
for dir in $ALL_DIRS; do
    cd $dir
    echo $dir
    `rm -f *.gcov *.gcno *.gcda`
done 
