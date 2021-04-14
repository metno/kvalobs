#! /bin/bash

image_name="postgres:9.6"
container_name="kvdb-9.6"
pgpassword=postgres
dbvolum=kvdb
name=$(docker ps -a --filter "name=$container_name" --format "{{.Names}}")

if [ $? -ne 0 ]; then
  echo "Failed to get the status information for container '$container_name'."
  exit 1
fi

if [ -n "$name" ]; then 
  name=$(docker ps --filter "name=$container_name" --format "{{.Names}}")

  if [ $? -ne 0 ]; then
    echo "Failed to get the status information for container '$container_name' (*)."
    exit 1
  fi

  if [ -n "$name" ]; then 
    echo "Container '$container_name' is running."
    exit 0
  else
    echo "Restarting container '$container_name'."
    if ! docker restart $container_name ; then 
      echo "Failed to restart '$container_name'."
      exit 1
    else 
      echo "Restarted '$container_name'."
      exit 0
    fi
  fi
fi

echo "Creating container '$container_name' using image '$image_name'."


#docker run 
#docker run --name some-postgres -e POSTGRES_PASSWORD=mysecretpassword -d postgres

docker run -d \
  -p 5435:5432 \
  -v $dbvolum:/var/lib/postgresql/data \
  --name $container_name \
  -e POSTGRES_PASSWORD=$pgpassword \
  -d \
  $image_name \
