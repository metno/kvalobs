#! /bin/bash

gitref () {
  repo=$(git remote show origin 2>/dev/null)

  if [ $? -eq 0 ]; then
    repo=$(echo "$repo" | grep -E "^ *Push +URL: " | sed -E 's/ *Push +URL:.+@//')
  else 
    repo=$(git config --get remote.origin.url )
 
    if [ $? -eq 0 ]; then
      repo=$(echo "$repo" | sed -E 's/.*@//')    
    else
      repo=$(basename $(git rev-parse --show-toplevel 2>/dev/null))
    fi
  fi
  ref=$(git describe --long 2>/dev/null )
  if [ $? -eq 0 ]; then
    ref="($ref) commit: $(git log -1 --format=format:'%H %aI %d')"
  else
    ref="(NO tags) commit: $(git log -1 --format=format:'%H %aI %d')"
  fi

  echo "$repo $ref"
}


ref=$(gitref)
echo "gitref written to GITREF"
echo "$ref"
echo "$ref" > GITREF

