#!/bin/bash

function syntax() {
	echo "$use $0 [--version=<TAG|commit-ID>] [--suffix=<n>] --file=<file.pcb>" >&2
	exit 1
}

function getVersion() {
	local commit="$(git log --pretty=oneline -n 1 --no-color |sed 's/[ \t]\+.*//')"
	local tag=""
	local version=""
	local -i out=1

	for tag in $(git tag --list)
	do
		[ "$(git show "$tag" |sed -n 's/^commit *//p')" = "$commit" ] && {
			out=0
			version="$tag"
			break
		}
	done
	[ -z "$version" ] && version="$commit"
	echo "$version"

	return $out
}

function errMsg() {
	echo "[ERROR!] $1" >&2
	[ $2 -ne 0 ] && exit $2
}
#-------------------------------------------------------------------------------------------------------------------------------
#                                                        M A I N
#-------------------------------------------------------------------------------------------------------------------------------
prjName="mbElectricalSystem"
targetDir=""
version=""
suffix=0
pcbFile=""
layers="F.Cu,B.Cu"

[ $# -gt 0 ] || syntax $0

for arg in $*
do
	if expr "$arg" : '^--[^=]\+=.\+' >/dev/null ; then
		key="${arg%%=*}"
		val="${arg#*=}"

		case "$key" in
			"--version")
				version="$val"
			;;
			"--suffix")
				suffix="$val"
			;;
			"--file")
				pcbFile="$val"
			;;
			*)
				errMsg "\"$arg\" is unknown argument" 0
				syntax $0
			;;
		esac
	else
		suntax $0
	fi
done

#
# Checking for the source file
#
[ -z "$pcbFile" ] && syntax $0
[ -f "$pcbFile" ] || errMsg "\"$pcbFile\" file not found" 127

#
# Retriving file's version information
#
[ -z "$version" ] && {
	version=$(getVersion) && echo "TAG:$version found!"
}

targetDir="$prjName-$version-Fabric-$suffix"

[ -e $targetDir ] && errMsg "\"$targetDir\" already exists" 129

mkdir --verbose $targetDir

kicad-cli pcb export drill   --output "$targetDir"                    "$pcbFile" || exit $?
kicad-cli pcb export gerbers --output "$targetDir" --layers "$layers" "$pcbFile" || exit $?
zip -r "$targetDir.zip" "$targetDir"

exit 0
