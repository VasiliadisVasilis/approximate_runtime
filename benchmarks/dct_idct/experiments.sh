# cat likwid_out | grep "Energy \[J\]," | cut -f2 -d ","
mkdir -p exps
ratios=`seq 0.0 0.1 1.0`
num=4

rm -f exps/output

for r in $ratios
do
	for i in `seq 0 $num`
	do
		file="exps/exp_${r}_${i}"
		echo $file
		./dct_idct  2048 2048 $r 23 2>&1 > $file
    duration=$(cat $file  | grep "Duration=" | cut -f2 -d "=")
    energy=$(cat $file  | grep "Energy=" | cut -f2 -d "=")
    psnr=$(cat $file | grep "PSNR" | cut -f2 -d "=")
    echo "${r}, ${i}, ${duration}, ${energy}, ${psnr}"
    echo "${r}, ${i}, ${duration}, ${energy}, ${psnr}" >> exps/output
	done
done
