#!/usr/bin/env bash
set -o errexit

# N4IRS 07/26/2017

#################################################
#                                               #
#                                               #
#                                               #
#################################################

# Takes one or two arguments. The first is the 2 digit hexadecimal address you are looking for
# the second is the i2c-dev bus number (defaults to 1).

bus=1
if [[ -n $2 ]]; then
    bus=$2
fi

mapfile -t data < <(i2cdetect -y $bus)

for i in $(seq 1 ${#data[@]}); do
    line=(${data[$i]})
    echo ${line[@]:1} | grep -q $1
    if [ $? -eq 0 ]; then
        echo "$1 is present."
        exit 0
    fi
done

echo "Not found."
exit 1
