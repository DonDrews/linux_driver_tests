#!/bin/bash

while [ 1 -eq  1 ]
do
	response=""
	read response
	echo $response > /dev/LCDchardev
done

