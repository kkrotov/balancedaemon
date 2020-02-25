#!/bin/bash
echo "Starting daemon ..."
./balancedaemon 8888 dbname=$1 
if [ $? -ne 0 ]
then
	echo "Error starting daemon"
	exit 1
fi

echo "Testing add balance operation ..."
RES=`echo "add ivanov 10" | netcat localhost 8888`
echo ${RES}

echo "Testing subtract balance operation ..."
RES=`echo "sub ivanov 10" | netcat localhost 8888`
echo $RES

echo "Finishing daemon process ..."
RES=`echo quit | netcat localhost 8888`
echo $RES
