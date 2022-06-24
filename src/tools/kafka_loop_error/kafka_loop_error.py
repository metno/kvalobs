#! /usr/bin/python3

from datetime import datetime, timedelta
from typing import Any, List, NoReturn
from subprocess import Popen, PIPE, run, STDOUT
from xmlrpc.client import Boolean
import regex
import sys
import signal
from time import sleep


reQabaseKafkaErr=regex.compile(r".*(kvQabased-[0-9]+).*[Kk]afka.+\(attempts.+#([0-9]+)")
verbose=False
def signalhandler( signal: int, frame: Any)->None:
  print(f"Got signal {signal}. Exit with status 0",flush=True)
  sys.exit(0)

def kill(name: str)->None:
  res=run(["/usr/bin/docker","kill", name], capture_output=True)
  if res.returncode == 0:
    print(f"Killed container {name}",flush=True)
  else:
    print(f"ERROR: {res.stdout}",flush=True)
    print(f"ERROR: {res.stderr}",flush=True)



def run_with_test()->NoReturn:
  #with Popen(["python3", "/home2/kvalobs/test.py"], stdout=PIPE, text=True, bufsize=1) as proc:
  with Popen(["python3", "test.py"], stdout=PIPE, text=True, bufsize=1) as proc:
    kafka_loop(proc)


def run_with_journal_kafka_error()->NoReturn:
  #with Popen(["/usr/bin/journalctl", "-u", "kvQabased@*.service", "--follow", "--grep", 'Could not send data to Kafka.'],stdout=PIPE, text=True, bufsize=1) as proc:
  with Popen(["/usr/bin/journalctl", "-u", "kvQabased@*.service", "--follow"],stdout=PIPE, text=True, bufsize=1) as proc:
    kafka_loop(proc)
  
def run_with_journal()->NoReturn:
  with Popen(["/usr/bin/journalctl", "-u", "kvQabased@*.service", "--follow"],stdout=PIPE, text=True, bufsize=1) as proc:
    while True:
      ret=proc.poll()
      if ret is not None:
        print(f"Subprocess has died with exit code {ret}")
        exit(1)
      line=proc.stdout.readline()
      print(line,flush=True)
      sleep(1)


def kafka_loop(proc: Popen)->NoReturn:
  print("----- Starting --------",flush=True)
  n=0
  deltaTime=timedelta(minutes=5)
  testTime=datetime.now()+deltaTime
  while True:
      ret=proc.poll()
      if ret is not None:
        print(f"Subprocess has died with exit code {ret}",flush=True)
        exit(1)
      line=proc.stdout.readline()
      n += 1
      line.rstrip()
      
      if verbose:
        print(line,flush=True)

      res=reQabaseKafkaErr.match(f"{line}")
      if res is None:
        now=datetime.now()
        if not verbose and now > testTime:
          print(f" ---- processed {n} log lines without kafka errors ----",flush=True)
          testTime=now+deltaTime
          n=0
        continue
      print(f" ---- processed {n} log lines without kafka errors ----",flush=True)
      testTime=now+deltaTime
      n=0
      print(" ----- Kafka loop error found! ----- ",flush=True)
      (name, count)=res.group(1,2)
      print(f"Container name: '{name}' error count: {count}",flush=True)
      if int(count) >= 10:
        kill(name)

def use():
  print('Use kafka_loop_error [--help | -h] [--verbose] [--journal-kafka-error | --journal | --test]',flush=True)
  
def isVerbose(args: List[str])->Boolean:
  count = sum([e=="--verbose" for e in args])
  return count>0 

def main():
  global verbose
  verbose=isVerbose(sys.argv)
  print(f"Verbose: {verbose}",flush=True)
  for cmd in sys.argv:
    if cmd == "--journal-kafka-error":
      print("Using journal looking for kafka errors.",flush=True)
      run_with_journal_kafka_error()
    elif cmd == "--journal":
      print("Using journal and print all kvQabased log to stdout",flush=True)
      run_with_journal()
    elif cmd == "--test":
      run_with_test()
    elif cmd == "--help" or cmd == "-":
      use()
      exit(0)
    
  use()
  print("Unknown cmd",flush=True)
  exit(1)


if __name__ == "__main__":
  signal.signal(signal.SIGINT, signalhandler)
  signal.signal(signal.SIGQUIT, signalhandler)
  main()
  

