#!/bin/bash
if [ -z $1 ]
then
	echo "USAGE: script <directory>"
	exit
fi
cd $1
original_trex_event1=10
original_trex_event2=11
trex_event_cnt=20
for dir_name in BC DE FG HI JK LM NO PQ RS TU;
do
	new_trex_event1=$trex_event_cnt
	new_trex_event2=$(($trex_event_cnt+1))
	mkdir -p $dir_name
	cd $dir_name
	for j in {1..100}
	do
		file_name=$dir_name-$j
		new_temp_constraint=$j
		cp ../original ./$file_name
		sed -i s/$original_trex_event1/$new_trex_event1/g $file_name
		sed -i s/$original_trex_event2/$new_trex_event2/g $file_name
		# 45 is the original value constraint on the temperature
		sed -i s/TEMPERATURE_VALUE_PLACEHOLDER/$new_temp_constraint/g $file_name
	done
	trex_event_cnt=$(($trex_event_cnt+2))
	cd ..
done

