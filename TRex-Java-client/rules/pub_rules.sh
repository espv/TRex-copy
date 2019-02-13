#!/usr/bin/env bash
rule_directory=$1
if [[ -z $rule_directory || ! -d $rule_directory ]]
then
	echo "USAGE: script <path to rule file>"
	exit
fi
cd $rule_directory
for dir in ./*;
do
    echo $dir
    if [[ -d $dir ]]
    then
        for rule in $dir/*; do
            java -jar ../../TRex-client.jar localhost 50254 -rule $dir/$rule
        done
    fi
done
