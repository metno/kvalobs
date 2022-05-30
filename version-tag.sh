#! /bin/bash

version=$(./configure --version | head -1 | cut -d' ' -f3)
git_hash=$(git log -1 --format=format:'%h')

echo "version: $version-$git_hash"