#! /bin/bash

export DOCKER_BUILDKIT=1

set -euo pipefail

kvuser=kvalobs
kvuserid=5010
mode=test
kafka_version=1.9.0-1.cflt~ubu20
kafka_version_jammy=1.8.0-1build1
VERSION="$(./version.sh)"
BUILDDATE=$(date +'%Y%m%d')
os=focal
registry="registry.met.no/met/obsklim/bakkeobservasjoner/data-og-kvalitet/kvalobs/kvbuild"
targets=
tag=latest
tag_and_latest="false"
kvbuild=
builddep=
kvcpp=
avalable_targets="kvdatainputd kvqabased kvmanagerd"
all=false
nocache=
builddep_tag=


use() {

  usage="\
Usage: $0 [--help] [--os os] [--staging|--prod|--test] [--kvbuild] [--buildep] [--tag-version] [--tag tag] [--tag-and-latest tag] [--tag-with-build-date] [--list]
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
  --tag-and-latest tagname 
                tag the image with the name tagname  and also create latest tag.
  --tag-with-build-date 
                tag with version and build date on the form version-YYYYMMDD 
                and set latest.
  --builddep-tag tag Use this builddep container
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
  --no-cache    Do not use the docker build cache.
  
  targets this is a list of targets to build. Available targets is:

  all builddep kvbuild kvcpp $avalable_targets

  The 'all' target builds: builddep kvbuild kvcpp $avalable_targets
  
"
echo -e "$usage\n\n"

}

while test $# -ne 0; do
  case $1 in
    --tag) 
        tag="$2"
        shift;;
    --tag-and-latest) 
        tag="$2"
        tag_and_latest=true
        shift;;
    --tag-version) 
        tag="$VERSION"
        tag_and_latest=true;;
    --tag-with-build-date) 
        tag="$VERSION-$BUILDDATE"
        tag_and_latest=true;;
    --buildep-tag) 
        builddep_tag="$2"
        shift;;
    --help) 
        use
        exit 0;;
    --os)
        os="$2"
        shift;;
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




if [ "$os" = "jammy" ]; then
  kafka_version="$kafka_version_jammy"
fi

if [ -z "$builddep_tag" ]; then
  builddep_tag="$tag"
fi

echo "tag: $tag"
echo "mode: $mode"
echo "os: $os"
echo "registry: $registry"
echo "kvbuild: $kvbuild"
echo "kvcpp: $kvcpp"
echo "builddep: $builddep"
echo "Targets: $targets"
echo "nocache: $nocache"
echo "kafka_version: ${kafka_version}"
echo "VERSION: $VERSION"
echo "builddep_tag: $builddep_tag"



chmod +x gitref.sh
./gitref.sh 

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
    kvcpp="kvcpp";
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


if [ -z "$targets" -a -z "$kvcpp" -a -z "$builddep" -a -z "$kvbuild" ]; then
  echo "No targets given."
  exit 1
fi


echo "Build targets: $builddep $kvbuild $kvcpp $targets"

if [ $mode = test ]; then 
  registry=""
  kvuserid=$(id -u)
else 
  registry="$registry/$mode/"
fi


# Must build the targets buildep and kvbuild first, if given.
pretargets="$builddep $kvbuild"
for target in $pretargets; do
  dockerfile="docker/kvalobs/${os}/${target}.dockerfile"
  echo "Building dockerfile: $dockerfile"
  docker build $nocache --build-arg "REGISTRY=${registry}" --build-arg "BASE_IMAGE_TAG=${tag}" --build-arg "kafka_VERSION=${kafka_version}" \
      --build-arg "BUILDDEP_TAG=${builddep_tag}" -f $dockerfile --tag "${registry}${target}:$tag" .
  
  if [ "$tag_and_latest" = "true" ]; then
      docker tag "${registry}${target}:$tag" "${registry}${target}:latest"
  fi

  if [ $mode != test ]; then 
    docker push ${registry}${target}:$tag
    if [ "$tag_and_latest" = "true" ]; then
      docker push "${registry}${target}:latest"
    fi
  fi
done


#Should we build the kvdev and kvruntime 
if [ -n "$kvcpp" ]; then
  dockerfile="docker/kvalobs/${os}/kvcpp.dockerfile"
  echo "Building dockerfile: docker/kvalobs/${os}/kvcpp.dockerfile"
  docker build $nocache --build-arg "REGISTRY=${registry}" --build-arg "BASE_IMAGE_TAG=${tag}" --build-arg "kafka_VERSION=${kafka_version}" \
    --build-arg "BUILDDEP_TAG=${builddep_tag}" -f $dockerfile --target dev --tag "${registry}kvcpp-dev:$tag" .
  docker build $nocache --build-arg "REGISTRY=${registry}" --build-arg "BASE_IMAGE_TAG=${tag}" --build-arg "kafka_VERSION=${kafka_version}" \
    --build-arg "BUILDDEP_TAG=${builddep_tag}" -f $dockerfile --target runtime --tag "${registry}kvcpp-runtime:$tag" .

  if [ "$tag_and_latest" = "true" ]; then
      docker tag "${registry}kvcpp-dev:$tag" "${registry}kvcpp-dev:latest"
      docker tag "${registry}kvcpp-runtime:$tag" "${registry}kvcpp-runtime:latest"
  fi

  if [ $mode != test ]; then 
    docker push ${registry}kvcpp-dev:$tag
    docker push ${registry}kvcpp-runtime:$tag

    if [ "$tag_and_latest" = "true" ]; then
      docker push "${registry}kvcpp-dev:latest"
      docker push "${registry}kvcpp-runtime:latest" 
    fi
  fi
fi

for target in $targets; do
  dockerfile="docker/kvalobs/${os}/${target}.dockerfile"

  echo "Building dockerfile: $dockerfile"
  docker build $nocache --build-arg "REGISTRY=${registry}" --build-arg "BASE_IMAGE_TAG=${tag}" --build-arg "kvuser=$kvuser" --build-arg "kvuserid=$kvuserid" --build-arg "kafka_VERSION=${kafka_version}" \
    --build-arg "BUILDDEP_TAG=${builddep_tag}" -f $dockerfile --tag ${registry}${target}:$tag .
  
  if [ "$tag_and_latest" = "true" ]; then
      docker tag "${registry}${target}:$tag" "${registry}${target}:latest"
  fi

  if [ $mode != test ]; then 
    docker push ${registry}${target}:$tag
    if [ "$tag_and_latest" = "true" ]; then
      docker push "${registry}${target}:latest"
    fi
  fi
done

