#!/bin/bash

build(){
	echo -e "\e[32m$1> cleaning started.\e[39m"
	(cd $1 && make clean)
}

build nprpc
build npc
build nplib
build npdbclient
build npdbstatic
build npdbserver
build npsys
build avr_info
build sim
build npserver

