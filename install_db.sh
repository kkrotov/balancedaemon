#!/bin/bash
psql template1 -U $2 -c "create database $1"
if [ $? -ne 0 ]
then
	echo "Error creating database $1"
	exit 1
fi
psql $1 -U $2 < balance.sql
if [ $? -ne 0 ]
then
	echo "Error restoring database dump"
	exit 1
fi
psql $1 -U $2 -c "ALTER FUNCTION public.decrement_balance(name text, val money) OWNER TO $2;" 
psql $1 -U $2 -c "ALTER FUNCTION public.increment_balance(name text, val money) OWNER TO $2;" 
psql $1 -U $2 -c "ALTER FUNCTION public.log_balance_changes() OWNER TO $2;" 
psql $1 -U $2 -c "ALTER TABLE public.operationlog OWNER TO $2;" 
psql $1 -U $2 -c "ALTER TABLE public.userbalance OWNER TO $2;" 
psql $1 -U $2 -c "ALTER TABLE public.userbalance_id_seq OWNER TO $2;" 

