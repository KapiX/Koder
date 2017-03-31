#!/bin/sh
function sort_catkeys {
	# sort_catkeys.sh from Humdinger
	tempfile=$1_orig
	mv $1 $tempfile
	head -1 $tempfile > $1
	tail -n +2 $tempfile | sort -k 2 -t$'\t' >> $1
	rm $tempfile
}

make catkeys
cd locales
sort_catkeys en.catkeys
for file in *.catkeys; do
	if [ "$file" == "en.catkeys" ]; then
		continue
	fi
	sort_catkeys $file
	awk -F'\t' -f ../merge_catkeys.awk $file en.catkeys > new_$file
	mv new_$file $file
	settype -t locale/x-vnd.Be.locale-catalog.plaintext $file
done
cd ..
