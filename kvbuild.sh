#! /bin/bash

export DOCKER_BUILDKIT=1

set -euo pipefail

kvuser=kvalobs
kvuserid=5010
mode=test

os=focal
#os=bionic
registry="registry.met.no/obs/kvalobs/kvbuild"
build=test
targets=
tag=latest
kvbuild=
builddep=
kvcpp=
avalable_targets="kvdatainputd kvqabased kvmanagerd"
all=false
nocache=

use() {

  usage="\
Usage: $0 [--help] [--staging|--prod|--test] [--kvbuild] [--buildep] [--tag tag] [--list]
          [--no-cache] targets

This script build kvalobs containers. 

If --staging or --prod is given it is copied to the 
container registry at $registry. 
If --test, the default, is used it will not be copied 
to the registry.


Options:
  --help        display this help and exit.
  --list        list targets.
  --tag tagname tag the image with the name tagname, default latest.
  --staging     build and push to staging.
  --prod        build and push to prod.
  --test        only build, default
  --kvbuild     Build kvalobs, ie the kvbuild container. 
  --kvcpp       Build kvcpp and kvruntime. kvdev has all kvcpp development files 
                and kvruntime has all runtime files needed for kvcpp
  --builddep    Build the buildep container. The dafault is not to buils
                the container. 
  --no-cache    Do not use the docker build cache.


  targets this is a list of targets to build. Available targets is:

  all builddep kvbuild kvdev $avalable_targets

"
echo -e "$usage\n\n"

}

while test $# -ne 0; do
  case $1 in
    --tag) 
        tag="$2"
        shift;;
    --help) 
        use
        exit 0;;
    --staging) mode=staging;;
    --prod) mode=prod;;
    --test) mode=test;;
    --kvbuild) kvbuild="kvbuild";;
    --builddep) builddep="builddep";;
    --kvcpp) kvcpp="kvcpp";;
    --list) 
        echo -e "\nTargets: all builddep kvbuild $avalable_targets\n\n"
        exit 0 ;;
    --no-cache) nocache="--no-cache";;
    -*) use
        echo "Invalid option $1"
        exit 1;;  
    *) targets="$targets $1";;
  esac
  shift
done


echo "tag: $tag"
echo "mode: $mode"
echo "os: $os"
echo "registry: $registry"
echo "kvbuild: $kvbuild"
echo "kvcpp: $kvcpp"
echo "builddep: $builddep"
echo "Targets: $targets"
echo "nocache: $nocache"

gitref.sh 

for target in $targets ; do
  found=false

  if [ $target = all ]; then 
    all=true;
    continue
  fi

  if [ $target = builddep ]; then 
    builddep="builddep";
    continue
  fi

  if [ $target = kvbuild ]; then 
    kvbuild="kvbuild";
    continue
  fi

  if [ $target = kvcpp ]; then 
    kvcpp="kcpp";
    continue
  fi


  for tmp in $avalable_targets ; do
    if [ $target = $tmp ]; then  
      found=true
    fi
  done
  
  if [ $found = false ]; then
   echo "Invalid target: $target"
   echo "Avalable targets: $avalable_targets"
   exit 1
  fi
done

if [ $all = true ]; then
  targets="$avalable_targets"
  builddep="builddep"
  kvbuild="kvbuild"
  kvcpp="kvcpp"
fi

targets="$builddep $kvbuild $targets"


if [ -z "$targets" -a -z "$kvcpp" ]; then
  echo "No targets given."
  exit 1
fi


echo "Build targets: $targets $kvcpp"

if [ $mode = test ]; then 
  registry=""
  kvuserid=$(id -u)
else 
  registry="$registry/$mode/"
fi

#Should we build the kvdev and kvruntime 
if [ -n "$kvcpp" ]; then 
  echo "Using dockerfile: docker/kvalobs/${os}/kvcpp.dockerfile"
  docker build $nocache --build-arg "REGISTRY=${registry}" --build-arg "BASE_IMAGE_TAG=${tag}" -f docker/kvalobs/${os}/kvcpp.dockerfile --target dev --tag "${registry}${os}-kvcpp-dev:$tag" .
  docker build $nocache --build-arg "REGISTRY=${registry}" --build-arg "BASE_IMAGE_TAG=${tag}" -f docker/kvalobs/${os}/kvcpp.dockerfile --target runtime --tag "${registry}${os}-kvcpp-runtime:$tag" .

  if [ $mode != test ]; then 
    docker push ${registry}${os}-kvcpp-dev:$tag
    docker push ${registry}${os}-kvcpp-runtime:$tag
  fi
fi

for target in $targets; do
  addArgs=false
  for tmp in $avalable_targets ; do
    if [ $target = $tmp ]; then
      addArgs=true
      break
    fi
  done

  dockerfile="docker/kvalobs/${os}/${target}.dockerfile"

  echo "Using dockerfile: $dockerfile"
  if [ $addArgs == true ]; then
    docker build $nocache --build-arg "REGISTRY=${registry}" --build-arg "BASE_IMAGE_TAG=${tag}" --build-arg "kvuser=$kvuser" --build-arg "kvuserid=$kvuserid" -f $dockerfile --tag ${registry}${os}-${target}:$tag .
  else
    docker build $nocache --build-arg "REGISTRY=${registry}" --build-arg "BASE_IMAGE_TAG=${tag}" -f $dockerfile --tag "${registry}${os}-${target}:$tag" .
  fi
  
  if [ $mode != test ]; then 
    docker push ${registry}${os}-${target}:$tag
  fi

done

