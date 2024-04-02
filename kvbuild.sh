#! /bin/bash

export DOCKER_BUILDKIT=1

set -euo pipefail

kvuser=kvalobs
kvuserid=5010
mode="test"
kafka_version=1.9.0-1.cflt~ubu20
kafka_version_jammy=1.8.0-1build1
VERSION="$(./version.sh)"
BUILDDATE=$(date +'%Y%m%d')
default_os="focal"
os=focal
registry="registry.met.no/met/obsklim/bakkeobservasjoner/data-og-kvalitet/kvalobs/kvbuild"
targets=
targets_in=
tag=latest
tag_and_latest="false"
avalable_targets="kvbuilddep kvbuild kvcpp kvdatainputd kvqabased kvmanagerd"
nocache=
build="true"
push="true"
use() {

  usage="\
Usage: $0 [--help] [--no-cache] [--os os] [--staging|--prod|--test] [--kvbuild] [--buildep] [--tag-version] [--tag tag] [--tag-and-latest tag] [--tag-with-build-date] [--list]
           [--all-targets] [--push-only] [--build-only] targets

This script build kvalobs containers. 

If --staging or --prod is given it is copied to the 
container registry at $registry. 
If --test, the default, is used it will not be copied 
to the registry.


Options:
  --help        display this help and exit.
  --list        list targets.
  --tag tagname tag the image with the name tagname, default latest.
  --tag-and-latest tagname 
                tag the image with the name tagname  and also create latest tag.
  --tag-with-build-date 
                tag with version and build date on the form version-YYYYMMDD 
                and set latest.
  --tag-version Use version from configure.ac as tag. Also tag latest.
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
  
  targets this is a list of targets to build. Available targets is:

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
        shift
        ;;
    --tag-and-latest) 
        tag="$2"
        tag_and_latest=true
        shift
        ;;
    --tag-version) 
        tag="$VERSION"
        tag_and_latest=true
        ;;
    --tag-with-build-date) 
        tag="$VERSION-$BUILDDATE"
        tag_and_latest=true
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
      nocache="--no-cache";;
    --build-only)
      push="false";;
    --push-only)
      build="false";;
    -*) use
        echo "Invalid option $1"
        exit 1
        ;;  
    *) targets_in="$targets_in $1";;
  esac
  shift
done

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
  kvuserid=$(id -u)
elif [ "$os" = "$default_os" ]; then
  registry="$registry/$mode/"
else 
  registry="$registry/$mode/$os/"
fi


echo "tag: $tag"
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
      -f "$dockerfile" --tag "${registry}${target}:$tag" .
  
    if [ "$tag_and_latest" = "true" ] && [ "$tag" != "latest" ]; then
      docker tag "${registry}${target}:$tag" "${registry}${target}:latest"
    fi
  done
fi


if [ "$push" = "true" ] && [ "$mode" != "test" ]; then
  for target in $targets; do
    echo "Pushing: ${registry}${target}:$tag"
    docker push "${registry}${target}:$tag"
    if [ "$tag_and_latest" = "true" ] && [ "$tag" != "latest" ]; then
      echo "Pushing: ${registry}${target}:latest"
      docker push "${registry}${target}:latest"
    fi
  done
fi
