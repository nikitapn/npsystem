#!/bin/bash

mkdir -p ./build
mkdir -p ./build/bin
mkdir -p ./build/lib

build(){
	echo -e "\e[32m$1> build started.\e[39m"
	(cd $1 && make)
	
	if [ $? -ne 0 ]; then
		echo -e "\e[31m$1> build failed.\e[39m"
		exit -1
	else
		echo -e "\e[32m$1> builded successfully.\e[39m\n"
	fi
}

build nplib
build nprpc 
build npc
build npdbclient
build npdbstatic
build npdbserver
build npsys
build avr_info
build sim
build npserver
build npwebserver

