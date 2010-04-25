#! /bin/bash

for i in {1..60}
do
   printf "#"
done  
#test suite
array=(jp20 jp21 jp22 jp23)

printf "\n### Loaded %d test files\n" ${#array[*]}
rank=0
n=0
score=0
for item in ${array[*]}
do
	printf "### Executing %s\n" $item
	./cl <$item >my$item execute
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
