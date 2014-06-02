for (( i = 0 ; i < 100 ; i++ ))
do
	./debug >log
	var=`grep "am sanity function" log|wc -l`
	var1=`grep "Hello task" log|wc -l`
	if [ $var -ne $var1 ];
	then
		echo "Sanity is executed $var times and tasks are $var1 times"
		exit
	fi
echo "finished $i"
done

echo success
