#!/bin/bash

MKCFILE="Makefile.dbgsyms"

symsList=" $(sed -n 's/^.* \(MBES_[^_]\+_DEBUG\) .*/\1/p' *.[ch] |sort |uniq)"

#
# Main symbols
#
echo "DEBUG          ?= 0" >  $MKCFILE
echo "MBES_KEEPTRACK ?= 0" >> $MKCFILE


#
# Module's symbols
#
for symbol in $symsList
do
	echo "$symbol ?= 0"  >> $MKCFILE
	echo "+ $symbol"
done
echo -e "\n"               >> $MKCFILE


echo "DBGSYMBOLS = \\"                                 >> $MKCFILE
echo -e "\t\"-DDEBUG=\$(DEBUG)\" \\"                   >> $MKCFILE
echo -e "\t\"-DMBES_KEEPTRACK=\$(MBES_KEEPTRACK)\" \\" >> $MKCFILE
for symbol in $symsList
do
	echo -e "\t\"-D${symbol}=\$($symbol)\" \\"  >> $MKCFILE
done
echo -e "\n"                                      >> $MKCFILE

echo "$MKCFILE file created"
