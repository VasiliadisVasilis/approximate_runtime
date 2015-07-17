# cat likwid_out | grep "Energy \[J\]," | cut -f2 -d ","
mkdir -p exps
ratios=`seq 0.0 0.1 1.0`
num=2
threads=7
rm -f exps/output

for r in $ratios
do
	for i in `seq 0 $num`
	do
		file="exps/exp_${r}_${i}"
		#echo "likwid-perfctr -C 1-$threads -m -O -g ENERGY ./sobel 10240 10240 $r $threads 2>&1 > $file"
		#likwid-perfctr -C 1-$threads -m -O -g ENERGY ./sobel 10240 10240 $r $threads 2>&1 > $file
		echo "./sobel 10240 10240 $r $threads 2>&1 > $file"
		./sobel 10240 10240 $r $threads 2>&1 > $file
    duration=$(cat $file  | grep "Duration," | cut -f2 -d ",")
    energy=$(cat $file  | grep "Energy," | cut -f2 -d ",")
    psnr=$(cat $file | grep "PSNR" | cut -f2 -d ",")
    echo "${r}, ${i}, ${duration}, ${energy}, ${psnr}"
    echo "${r}, ${i}, ${duration}, ${energy}, ${psnr}" >> exps/output
	done
done
