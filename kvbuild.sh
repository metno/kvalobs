#! /bin/bash

export DOCKER_BUILDKIT=1

set -euo pipefail

kvuser=kvalobs
kvuserid=5010
mode="test"
kafka_version=2.3.0-1build2
kafka_version_jammy=1.8.0-1build1
VERSION="$(./version.sh)"
BUILDDATE=$(date +'%Y%m%d')
default_os="noble"
os="noble"
registry="registry.met.no/met/obsklim/bakkeobservasjoner/data-og-kvalitet/kvalobs/kvbuild"
targets=
targets_in=
tag="${VERSION}"
avalable_targets="kvbuilddep kvbuild kvcpp kvdatainputd kvqabased kvmanagerd"
nocache=
build="true"
push="true"
KV_BUILD_DATE="${KV_BUILD_DATE:-}"
tags=""
tag_counter=0

if [ -n "${KV_BUILD_DATE}" ]; then
  BUILDDATE=$KV_BUILD_DATE
fi

use() {

  usage="\
Usage: $0 [--help] [options] targets

This script build kvalobs containers. 

If --staging or --prod is given it is copied to the 
container registry at $registry. 
If --test, the default, is used it will not be copied 
to the registry.


Options:
  --help        display this help and exit.
  --list        list targets.
  --tag tagname tag the image with tagname, default ${VERSION}.
  --tag-and-latest tagname 
                tag the image with tagname and also create latest tag.
  --tag-with-build-date 
                Creates three tags: ${VERSION}, latest and a ${VERSION}-${BUILDDATE}.
                If the enviroment variable KV_BUILD_DATE is set use
                this as the build date. Format KV_BUILD_DATE YYYYMMDD.
  --staging     build and push to staging.
  --prod        build and push to prod.
  --test        only build, default
  --kvbuild     Build kvalobs, ie the kvbuild container. 
  --kvcpp       Build kvcpp and kvruntime. kvdev has all kvcpp development files 
                and kvruntime has all runtime files needed for kvcpp
  --builddep    Build the buildep container. The dafault is not to build
                the container. 
  --os os       The os to build for. There must exist deinition files in docker/kvalobs/os
                Default: $os
  --build-only  Only build the container(s). No push. Default: $build
  --push-only   Push only the container(s). No build. Default: $push
                The containers must prevosly been buildt whith the same flags.
  --no-cache    Do not use the docker build cache.
  --all-targets Build all targets.
  --print-version-tag
                Print the version tag and exit.

              
  The following opptions is mutally exclusive: --tag, --tag-and-latest, --tag-with-build-date
  The following options is mutally exclusive: --build-only, --push-only
  The following options is mutally exclusive: --staging, --prod, --test
  
  This is a list of targets to build. Available targets is:

    '$avalable_targets'

"
echo -e "$usage\n\n"

}

validate_targets() {
  local target_to_validate=$1

  for target_in in $target_to_validate ; do
    found=false
    for target in $avalable_targets ; do
      if [ "$target_in" = "$target" ]; then
        found=true
        break
      fi
    done 
    if [ "$found" == "false" ]; then
      echo "Invalid target '$targets_in'"
      exit 1
    fi
  done
}

while test $# -ne 0; do
  case $1 in
    --tag) 
        tag="$2"
        tag_counter=$((tag_counter + 1))
        shift
        ;;
    --tag-and-latest) 
        tag="$2"
        tags="latest"
        tag_counter=$((tag_counter + 1))
        shift
        ;;
    --tag-with-build-date) 
        tag_counter=$((tag_counter + 1))
        tags="latest $VERSION-$BUILDDATE"
        ;;
    --all-targets)
      targets_in="$avalable_targets"
      ;;
    --help) 
        use
        exit 0;;
    --os)
        os="$2"
        shift
        ;;
    --staging) mode="staging";;
    --prod) mode="prod";;
    --test) mode="test";;
    --kvbuild) 
        targets_in="$targets_in kvbuild";;
    --builddep) 
        targets_in="$targets_in kvbuilddep";;
    --kvcpp) 
        targets_in="$targets_in kvcpp";;
    --list) 
        echo -e "\nTargets: all $avalable_targets\n\n"
        exit 0 
        ;;
    --no-cache) 
        nocache="--no-cache"
        ;;
    --build-only)
        push="false"
        ;;
    --push-only)
        build="false"
        ;;
    --print-version-tag)
      echo "$VERSION-$BUILDDATE"
      exit 0;;
    -*) use
        echo "Invalid option $1"
        exit 1
        ;;  
    *) targets_in="$targets_in $1";;
  esac
  shift
done

if [ $tag_counter -gt 1 ]; then
  echo "Only one of --tag, --tag-and-latest or --tag-with-build-date can be used"
  exit 1
fi

if [ "$push" = false ] && [ "$build" = false ]; then
  echo "Either --build-only or --push-only must be used"
  exit 1
fi

if [ "$os" = "jammy" ]; then
  kafka_version="$kafka_version_jammy"
fi

validate_targets "$targets_in"

# Make the target build in correct sequence, ie kvbuilddep, kvbuild, kvcpp, etc ...
# The correct sequence is given by avalable_targets.
for target in $avalable_targets ; do
  for target_in in $targets_in ; do
    if [ "$target" = "$target_in" ]; then
      if [ "$target" = "kvcpp" ]; then
        targets="$targets kvcpp-dev kvcpp-runtime"
      else
        targets="$targets $target"
      fi
      break
    fi
  done
done

if [ $mode = test ]; then 
  registry="$os/"
    
elif [ "$os" = "$default_os" ]; then
  registry="$registry/$mode/"
else 
  registry="$registry/$mode-$os/"
fi


echo "tag: $tag"
echo "tags: $tags"
echo "targets_in: $targets_in"
echo "Targets: $targets"
echo "mode: $mode"
echo "os: $os"
echo "registry: $registry"
echo "nocache: $nocache"
echo "kafka_version: ${kafka_version}"
echo "build: $build"
echo "push: $push"
echo "VERSION: $VERSION"

chmod +x gitref.sh
./gitref.sh 

echo "Build targets: $targets"

if [ "$build" = "true" ]; then
  for target in $targets; do
    dockerfile="docker/kvalobs/${os}/${target}.dockerfile"
    echo "Building dockerfile: $dockerfile"
    docker build $nocache --build-arg "REGISTRY=${registry}" --build-arg "BASE_IMAGE_TAG=${tag}" \
      --build-arg "kvuser=$kvuser" --build-arg "kvuserid=$kvuserid" \
      --build-arg "kafka_VERSION=${kafka_version}" \
      -f "$dockerfile" --tag "${registry}${target}:${tag}" .
  
    for tagname in $tags; do
      echo "Tagging: ${registry}${target}:$tagname"
      docker tag "${registry}${target}:$tag" "${registry}${target}:$tagname"
    done
  done
fi


if [ "$push" = "true" ] && [ "$mode" != "test" ]; then
  for target in $targets; do
    echo "Pushing: ${registry}${target}:$tag"
    docker push "${registry}${target}:$tag"
    for tagname in $tags; do
      echo "Pushing: ${registry}${target}:$tagname"
      docker push "${registry}${target}:$tagname"
    done
  done
fi
