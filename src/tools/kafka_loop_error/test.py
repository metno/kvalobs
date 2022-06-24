import sys
from time import sleep

list=[
  "juni 23 06:43:34 kvprod-kvalobs-int-b kvQabased-4[3648808]: -- Could not send data to Kafka. Send queue size=1. Retrying (attempts #0) ...",
  "juni 23 06:43:34 kvprod-kvalobs-int-b kvQabased-7[3648816]: -- Could not send data to Kafka. Send queue size=1. Retrying (attempts #0) ...",
  "juni 23 06:43:34 kvprod-kvalobs-int-b kvQabased-9[3648924]: -- Could not send data to Kafka. Send queue size=1. Retrying (attempts #0) ...",
  "juni 23 06:43:34 kvprod-kvalobs-int-b kvQabased-11[3648874]: -- Could not send data to Kafka. Send queue size=1. Retrying (attempts #0) ...",
  "juni 23 06:43:34 kvprod-kvalobs-int-b kvQabased-0[3648832]: -- Could not send data to Kafka. Send queue size=1. Retrying (attempts #0) ...",
  "juni 23 06:43:34 kvprod-kvalobs-int-b kvQabased-5[3648900]: -- Could not send data to Kafka. Send queue size=1. Retrying (attempts #0) ...",
  "juni 23 06:43:34 kvprod-kvalobs-int-b kvQabased-12[3648917]: -- Could not send data to Kafka. Send queue size=1. Retrying (attempts #0) ...",
  "juni 23 06:43:34 kvprod-kvalobs-int-b kvQabased-8[3648893]: -- Could not send data to Kafka. Send queue size=1. Retrying (attempts #0) ...",
  "juni 23 06:43:35 kvprod-kvalobs-int-b kvQabased-10[3648821]: -- Could not send data to Kafka. Send queue size=1. Retrying (attempts #0) ...",
  "juni 23 06:43:36 kvprod-kvalobs-int-b kvQabased-6[3648876]: -- Could not send data to Kafka. Send queue size=1. Retrying (attempts #0) ...",
  "juni 23 06:43:50 kvprod-kvalobs-int-b kvQabased-10[3648821]: -- Could not send data to Kafka. Send queue size=1. Retrying (attempts #10) ..."
]


def main():
  while True:
    for l in list:
      print(l, flush=True)
      sleep(1)
    sleep(2)

if __name__ == "__main__":
  try:
    main()
  except KeyboardInterrupt:
    pass