#! /bin/bash

version=$(./version.sh)
git_hash=$(git log -1 --format=format:'%h')
version=$(echo $version | sed -e 's/~/_/')

echo "$version-$git_hash"