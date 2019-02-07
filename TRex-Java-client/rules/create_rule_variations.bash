#!/bin/bash
rule_directory=$1
if [[ -z $rule_directory || ! -d $rule_directory ]]
then
	echo "USAGE: script <path to rule file>"
	exit
fi
cd $1
trex_event_cnt=2
for dir_name in BC DE FG HI JK LM NO PQ RS TU;
do
	new_trex_event_temp=$trex_event_cnt
	new_trex_event_humidity=$(($trex_event_cnt+1))
	mkdir -p $dir_name
	cd $dir_name
	for j in {1..100}
	do
		file_name=$dir_name-$j
		new_temp_constraint=$j
		cp ../original ./$file_name
		sed -i s/INCOMING_EVENT_TEMP/$new_trex_event_temp/g $file_name
		sed -i s/INCOMING_EVENT_HUMIDITY/$new_trex_event_humidity/g $file_name
		# 45 is the original value constraint on the temperature
		sed -i s/TEMPERATURE_VALUE_PLACEHOLDER/$new_temp_constraint/g $file_name
	done
	trex_event_cnt=$(($trex_event_cnt+2))
	cd ..
done
