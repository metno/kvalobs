#! /bin/bash
container_name="etcd-v3.3"

name=$(docker ps --filter "name=$container_name" --format "{{.Names}}")

if [ $? -ne 0 ]; then
  echo "Failed to get the status information for container '$container_name'."
  exit 1
fi

if [ -z "$name" ]; then 
  echo "Container '$name' is not running."
  echo "Try to start it with 'start_etcd.sh'"
  exit 1
fi

#docker exec $name /bin/sh -c "/usr/local/bin/etcd --version"
docker exec $name /bin/sh -c "ETCDCTL_API=3 /usr/local/bin/etcdctl $(echo $@)"
