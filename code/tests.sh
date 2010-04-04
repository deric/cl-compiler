#! /bin/bash

for i in {1..60}
do
   printf "#"
done  
#test suite
array=(jp0 jp01 jp02 jp03 jp04 jp05)

printf "\n### Loaded %d test files\n" ${#array[*]}

for item in ${array[*]}
do
	printf "### Executing %s\n" $item
	./cl <$item >my$item
	colordiff s$item my$item
done
echo "#### Tests completed ####"

