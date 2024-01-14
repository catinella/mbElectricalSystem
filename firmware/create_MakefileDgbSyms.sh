#!/bin/bash

MKCFILE="Makefile.dbgsyms"

symsList=" $(sed -n 's/^.* \(MBES_[^_]\+_DEBUG\) .*/\1/p' *.[ch] |sort |uniq)"

echo "DEBUG ?= 0"            > $MKCFILE
for symbol in $symsList
do
	echo "$symbol ?= 0"    >> $MKCFILE
done
echo -e "\n"                 >> $MKCFILE


echo "DBGSYMBOLS = \\"                            >> $MKCFILE
echo -e "\t\"-DDEBUG=\$(DEBUG)\" \\"              >> $MKCFILE
for symbol in $symsList
do
	echo -e "\t\"-D${symbol}=\$($symbol)\" \\"  >> $MKCFILE
done
echo -e "\n"                                      >> $MKCFILE

echo "$MKCFILE file created"
