#!/bin/zsh

set -eu

typeset -a w_clang
zparseopts -D -E -- -with-clang=w_clang

if [[ $# != 0 ]]; then
	echo "Usage: $0 [--with-clang]"
	exit 1
fi

if [[ ! -d llvm ]]; then
	git clone https://github.com/ginsbach/llvm
fi
cd llvm
git checkout reductions
if [[ ${#w_clang} -gt 0 ]]; then
	cd projects
	if [[ ! -d clang ]]; then
		git clone https://github.com/ginsbach/clang
	fi
	cd clang
	git checkout research
	cd ../../..
fi

echo " Downloaded without clang.  Use --with-clang to build that too"

echo "llvm/lib/IDLParser/Idioms.idl contains the IDL code"
echo "llvm/lib/IDLPasses/CustomPassReplace does stuff with the idioms"
