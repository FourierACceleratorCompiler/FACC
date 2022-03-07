#!/bin/bash

for inffile in $(find -name INFO); do
	pushd $(dirname $inffile)
	repo=$(grep -e Source INFO | cut -f2- -d:)
	commit=$(grep -e Commit INFO | cut -f2- -d:)
	echo "Repo is $repo"
	rm -rf code
	(git clone --quiet $repo code
	ls
	cd code
	git checkout $commit
	# Now, get out the right region.
	if [[ -f ../EXTRACT ]]; then
		../../extractor.sh ../EXTRACT > ../self_contained_code.c
		if [[ -f ../EXTRACT_LIBRARY ]]; then
			../../extractor.sh ../EXTRACT_LIBRARY > ../context_code.c
		fi

		cd ..
		make
	else
		echo "No extraction file present, so not sure what regions of code to check for $repo"
	fi
	) &
	popd
done

wait
