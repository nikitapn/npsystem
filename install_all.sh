#!/bin/bash

build(){
	echo -e "\e[32m$1> installing...\e[39m"
	(cd $1 && make install)
}

build nprpc
build nplib
build sim
build npdbclient
build npdbserver
build npserver