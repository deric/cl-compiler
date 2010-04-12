#! /bin/bash

for i in {1..60}
do
   printf "#"
done  
#test suite
array=(jp0 jp01 jp02 jp03 jp04 jp05 jp06 jp07 jp08 jp09 jp10 jp11 jp12)

printf "\n### Loaded %d test files\n" ${#array[*]}
rank=0
n=0
score=0
for item in ${array[*]}
do
	printf "### Executing %s\n" $item
	./cl <$item >my$item
        colordiff s$item my$item
	res=`diff s$item my$item | wc -l`
	if [ "$res" -eq 0 ] 
	then
  		rank=`expr $rank + 1`
		echo "MATCH"
	fi
	n=`expr $n + 1`
	score=`expr $score + $res`
	echo $output
done
echo "#### Tests completed ####"
printf "\n Rank %d/%d (score: %d)\n" $rank $n $score
