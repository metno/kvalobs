#! /bin/bash

export DOCKER_BUILDKIT=1

set -euo pipefail

user=$(id -un)
userid=$(id -u)
groupid=$(id -g)
os="noble"
builddep=false
build_home=false
registry=
nocache=
run=false
build_targets=
name="kvbuild"
tag="latest"
registry=

use() {

  usage="\
Usage: $0 [--help] [options] targets

This build a container that can be used for building kvalobs in a container. 


Options:
  --help        display this help and exit.
  --os os       The os to build for. There must exist definition files in docker/kvalobs/${os}
                
  --builddep    Build the kvbuilddep container. Also force rebuild of the kvbuild-home container.
                The container is automatically build if it does not exist in the local 
                docker registry.
  --build-home  Force rebuild of the kvbuild-home container. Automatically builds 
                if it does not exist locally.
  --nocache     Build the container without using cache.
  --run         Run the container. The default is not to run the container.
                The container will be built and run if it does not exist locally.

"
echo -e "$usage\n\n"

}


while test $# -ne 0; do
  case $1 in
    --builddep) 
        builddep=true
        build_home=true
        ;;
    --build-home) 
        build_home=true
        ;;
    --nocache) 
        nocache="--no-cache"
        ;;
    --help) 
        use
        exit 0;;
    --os)
        os="$2"
        shift
        ;;
    --no-cache) 
        nocache="--no-cache"
        ;;
    --run)
        run=true
        ;;
    -*) use
        echo "Invalid option $1"
        exit 1
        ;;  
    *)  use
        echo "Invalid option $1"
        exit 1
  esac
  shift
done


kvbuilddep_exists() {
  if ! docker image inspect "${os}/kvbuilddep:latest" > /dev/null 2>&1; then
    echo "kvbuilddep container does not exist locally."
    builddep=true
    build_home=true
  fi
}

kvbuild_local_exists() {
  if ! docker image inspect "${os}/kvbuild-home:latest" > /dev/null 2>&1; then
    echo "kvbuild-home container does not exist locally."
    build_home=true
  fi
}

kvbuilddep_exists
kvbuild_local_exists

if [ "$build_home" = true ]; then
  build_targets="kvbuild-home"
fi

if [ "$builddep" = true ]; then
  build_targets="kvbuilddep ${build_targets}"
fi

name="${name}-${os}"

echo "tag: $tag"
echo "name: $name"
echo "build_targets: $build_targets"
echo "os: $os"
echo "nocache: $nocache"
echo "build_home: $build_home"
echo "builddep: $builddep"
echo "user: $user"
echo "userid: $userid"
echo "groupid: $groupid"
echo "run: $run"

for target in $build_targets; do
  echo "Will build target: $target"
  echo "docker build $nocache \
    --build-arg user=$user --build-arg userid=$userid --build-arg groupid=$groupid \
    --build-arg REGISTRY=$os \
    -f \"docker/kvalobs/${os}/${target}.dockerfile\" --tag \"${os}/${target}:${tag}\"" .
  docker build $nocache \
    --build-arg "user=$user" --build-arg "userid=$userid" --build-arg "groupid=$groupid" \
    --build-arg "REGISTRY=$os" \
    -f "docker/kvalobs/${os}/${target}.dockerfile" --tag "${os}/${target}:${tag}" .
done


if [ "$builddep" = true ] || [ "$build_home" = true ]; then
  echo "Stopping and removing container: ${name}"
  if docker stop ${name} > /dev/null 2>&1; then
    echo "Successfully stopped container: ${name}"
  fi
  if docker rm ${name} > /dev/null 2>&1; then
    echo "Successfully removed container: ${name}"
  fi
fi

if [ "$run" = true ]; then
  ret=$(docker ps -q -a -f "name=${name}" -f "status=exited")
  if [ -n "${ret}" ]; then
    echo "Container $name is stopped. Restaring .... "
    exec docker start -ia $name
  fi

  ret=$(docker ps -q -a -f "name=${name}" -f "status=running")
  if [ -n "${ret}" ]; then
    echo "Container $name is running. connecting .... "
    exec docker exec -it -u ${userid}:${groupid} $name /bin/bash
  fi

  echo "Creating container $name, from image ${os}/kvbuild-home:${tag}"
  mydir="$(pwd)"
  exec docker run -ti -v ${mydir}:/workspaces/kvalobs --name ${name} ${os}/kvbuild-home:${tag}
fi
   
