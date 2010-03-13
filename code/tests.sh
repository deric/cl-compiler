#! /bin/bash

for i in {1..60}
do
   printf "#"
done  
#test suite
array=(jp0 jp01 jp02)

printf "\n### Loaded %d test files\n" ${#array[*]}

for item in ${array[*]}
do
	printf "### Executing %s\n" $item
	./cl <$item >tmp
	colordiff s$item tmp
done

rm tmp
echo "#### Tests completed ####"

