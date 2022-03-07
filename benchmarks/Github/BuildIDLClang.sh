#!/bin/bash

set -eux

llvm() {
	with_clang=$1
	typeset -a flags
	flags=()
	if [[ $with_clang == True ]]; then
		flags=( -DLLVM_ENABLE_PROJECTS=clang)
	fi
	echo $flags
	cmake ../llvm -G Ninja -DCMAKE_BUILD_TYPE=Release -DBUILD_SHARED_LIBS=ON -DCMAKE_INSTALL_PREFIX="$PWD/../llvm-install" $flags
}

clang() {
	cmake ../llvm -G Ninja -DBUILD_SHARED_LIBS=ON -DCMAKE_INSTALL_PREFIX="$PWD/../clang-install"
}

mkdir -p llvm-build
cd llvm-build
llvm True
ninja -j 10
