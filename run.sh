for (( i = 0 ; i < 1000 ; i++ ))
do
	./debug >log
	var=`grep "sanity function Mytask" log|wc -l`
	var1=`grep "Hello task" log|wc -l`
	if [ $var -ne $var1 ];
	then
		echo "Sanity is executed $var times and tasks are $var1 times"
	fi
done
