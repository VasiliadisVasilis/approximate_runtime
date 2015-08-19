# cat likwid_out | grep "Energy \[J\]," | cut -f2 -d ","
mkdir -p exps
mkdir -p plots
ratios=`seq 0.0 0.1 1.0`
num=4
threads=23
numpics=100
rm -f exps/output

for r in $ratios
do
	for i in `seq 0 $num`
	do
		file="exps/exp_${r}_${i}"
		echo "./fisheye -s $numpics -f 40 -w 640 -h 480 -t $threads -R $r image.bmp 2>&1 > $file"
		./fisheye image.bmp -s $numpics -f 40 -x 1296 -y 972 -w 640 -h 480 -t $threads -R $r 2>&1 > $file
    duration=$(cat $file  | grep "Duration=" | cut -f2 -d "=")
    energy=$(cat $file  | grep "Energy=" | cut -f2 -d "=")
    psnr=$(./psnr out.bmp correct.bmp | grep "ERROR=" | cut -f2 -d "=")
    echo "${r}, ${i}, ${duration}, ${energy}, ${psnr}"
    echo "${r}, ${i}, ${duration}, ${energy}, ${psnr}" >> exps/output
	done
done

../average_exps 5 $(($num+1)) < exps/output > plots/output
