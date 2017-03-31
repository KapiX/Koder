{
	if(FNR == NR) {
		if(FNR > 1) { a[$1 FS $2 FS $3] = $0 }
	} else {
		if($1 FS $2 FS $3 in a) {
			print a[$1 FS $2 FS $3]
		} else {
			print $0
		}
	}
}