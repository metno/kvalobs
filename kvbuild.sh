#! /bin/bash

export DOCKER_BUILDKIT=1

set -euo pipefail

kvuser=kvalobs
kvuserid=5010
mode=test
kafka_version=1.9.0-1.cflt~ubu20
kafka_version_jammy=1.8.0-1build1
VERSION="$(./version.sh)"

os=focal
registry="registry.met.no/met/obsklim/kvalobs/kvbuild"
build=test
targets=
tag=latest
tag_and_latest="false"
kvbuild=
builddep=
kvcpp=
avalable_targets="kvdatainputd kvqabased kvmanagerd"
all=false
nocache=
src=
dst=
copy_dryrun=
builddep_tag=


use() {

  usage="\
Usage: $0 [--help] [--os os] [--staging|--prod|--test|--copy src dst|--copy-dry-run] [--kvbuild] [--buildep] [--tag-version] [--tag tag] [--tag-and-latest tag] [--list]
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
  --tag-and-latest tagname tag the image with the name tagname  and also create latest tag.
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
  --copy src dst Copy from src repositoy to dst repository.
  --copy-dry-run Same as --copy, but only list the actions that will be taken.


  targets this is a list of targets to build. Available targets is:

  all builddep kvbuild kvdev $avalable_targets

  copy:
    src: registry:container_name:tag
    dst: registry:container_name:tag

    Regestry must be a host (dns or ip) or one of prod, staging or test. If registry
    is empty 'test' is asumed. 

    src: container_name can be empty or a container_name. If empty the folowwing containers are
    assumed, kvcpp-runtime, $avalable_targets.

    dst: container_name is not empty a \"reaname\" is implied. In this case ony one container_name is 
    valid for the src container_name.

    ex 1.
      kvbuild --copy staging:: prod:: 

      This copies the latest (tag) from staging to the latest(tag) in prod for:  kvcpp-runtime, $avalable_targets.

    ex 2.
      kvbuild --copy staging:: prod::7.0.0 

      This copies the latest (tag) from staging to the tag 7.0.0 in prod for:  kvcpp-runtime, $avalable_targets.

    ex 3.
      kvbuild --copy staging:kvqabased: prod:: 

      This copies the kvqabased latest (tag) from staging to prod latest (tag).
    
    ex 4.
      kvbuild --copy prod:: test:: 

      This copies the latest (tag) from prod to local docker latest (tag).

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
    --copy)
      mode=copy
      src="$2"
      shift
      dst="$2"
      shift;; 
    --copy-dry-run)
      mode=copy
      src="$2"
      shift
      dst="$2"
      copy_dryrun=true
      shift;; 
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


_decode_dst_paths() {
  _dp_registry=
  _dp_target=$(echo $1 | cut -d: -f2)
  _dp_tag=

  local cp1=$(echo $1 | cut -d: -f1)
  local cp3=$(echo $1 | cut -d: -f3)
  local tag=
  
  if [ "$cp1" = "prod" ]; then 
    _dp_registry="$registry/prod/"
  elif [ "$cp1" = "staging" ]; then
    _dp_registry="$registry/staging/"
  elif [ "$cp1" = "test" -o -z "$cp1" ]; then
    _dp_registry=""
  else 
    _dp_registry="$cp1/"
  fi

  
  if [ "$cp3" = "latest" -o -z "$cp3" ]; then
    _dp_tag="latest"
  else
    _dp_tag="$cp3"
  fi

  return 0
}

  

_decode_src_paths() {
  _dp_registry=
  _dp_targets=
  _dp_tag=

  local cp1=$(echo $1 | cut -d: -f1)
  local cp2=$(echo $1 | cut -d: -f2)
  local cp3=$(echo $1 | cut -d: -f3)
  local tag=
  local res=

  if [ "$cp1" = "prod" ]; then 
    _dp_registry="$registry/prod/"
  elif [ "$cp1" = "staging" ]; then
    _dp_registry="$registry/staging/"
  elif [ "$cp1" = "test" -o -z "$cp1" ]; then
    _dp_registry=""
  else 
    _dp_registry="$cp1/"
  fi

  if [ "$cp2" = "all" -o -z "$cp2" ]; then
    targets="kvcpp-runtime $avalable_targets"
  else
    targets="$cp2"
  fi

  if [ "$cp3" = "latest" -o -z "$cp3" ]; then
    _dp_tag="latest"
  else
    _dp_tag="$cp3"
  fi

  for target in $targets ; do
    _dp_targets="$_dp_targets $target"
  done
  return 0
}

copy() {
  local src_reg=
  local src_targets=
  local src_tag
  local dst_reg=
  local dst_targets=
  local dst_tag
  mkdir -p /tmp/kvbuild
  _decode_src_paths $1
  src_reg=$_dp_registry
  src_targets=$_dp_targets
  src_tag=$_dp_tag
  _decode_dst_paths $2
  dst_reg=$_dp_registry
  dst_target=$_dp_target
  dst_tag=$_dp_tag

  
  if [ -n "$dst_target" ]; then
    n=$(echo $src_targets | wc -w)
    if [ "$n" -ne 1 ]; then 
      echo
      echo "When dst target '$dst_target' is given. Then src target must be exactly one element" 
      echo
      exit 1
    fi
  fi

  for target in $src_targets ; do 
    src="${src_reg}${target}:${src_tag}" 
    if [ -n "${dst_target}" ]; then
      dst="${dst_reg}${dst_target}:${dst_tag}"
    else 
      dst="${dst_reg}${target}:${dst_tag}"
    fi


    if [ -n "$copy_dryrun" ]; then
      echo "dryrun: $src -> $dst"
      continue
    fi

    if [ -n "$dst_reg" ]; then 
      docker pull $src &>/tmp/kvbuild/docker_err

      if [ "$?" -ne 0 ]; then
        echo "FAILED to pull: '$src'"
        cat /tmp/kvbuild/docker_err
        continue
      fi
    fi

    docker tag $src $dst &>/tmp/kvbuild/docker_err

    if [ "$?" -ne 0 ]; then
      echo "FAILED to tag: $src -> $dst"
      cat /tmp/kvbuild/docker_err
      continue
    else
      echo "Ok - $src -> $dst"
    fi

    if [ -z "$dst_reg" ]; then
      continue;
    fi 
    docker push $dst &>/tmp/kvbuild/docker_err

    if [ "$?" -ne 0 ]; then
      echo "FAILED to push: '$dst'"
      cat /tmp/kvbuild/docker_err
      continue
    fi

  done
}


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

if [ "$mode" = "copy" ]; then
  if [ -z "$src" -o -z "$dst" ]; then
    echo
    echo "Copy need a 'src' and 'dst' argument."
    echo
    exit 1
  fi
  copy $src $dst
  exit 0
fi


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

