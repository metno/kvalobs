#include "pgqueue.h"

#include <chrono>
#include <csignal>
#include <exception>
#include <iostream>
#include <string>
#include <thread>
#include <vector>
#include <getopt.h>

namespace {
volatile std::sig_atomic_t stopRequested = 0;

void requestStop(int) { stopRequested = 1; }


void usage(const char *program) {
  std::cerr << "Usage: " << program << " [OPTIONS] [CONNECTIONS...]\n";
  std::cerr << "Options:\n";
  std::cerr << "  -e, --environment ENV    Set the environment (default: development)\n";
  std::cerr << "  -r, --raw                Konsumes the raw queue\n";
  std::cerr << "  -c, --consumer CONSUMER  Set the consumer (default: kvconsumer)\n";
  std::cerr << "  -h, --help               Show this help message\n\n";
}

struct Options {
  std::string environment;
  std::string topic;
  std::string topicLowpri;
  std::string consumer;
  bool raw;
  std::vector<std::string> connections;
};


Options parse_options(int argc, char **argv) {
  Options options;
  static struct option long_options[] = {
      {"environment", required_argument, nullptr, 'e'},
      {"consumer", required_argument, nullptr, 'c'},
      {"raw", no_argument, nullptr, 'r'},
      {"help", no_argument, nullptr, 'h'},
      {nullptr, 0, nullptr, 0}
  };
  int option_index = 0;
  int opt;
  options.raw=false;
  while ((opt = getopt_long(argc, argv, "e:c:h:r", long_options, &option_index)) != -1) {
    switch (opt) {
    case 'e':
      options.environment = optarg;
      break;
    case 'c':
      options.consumer = optarg;
      break;
    case 'r':
      options.raw = true;
      break;
    case 'h':
      usage(argv[0]);
      exit(0);
    default:
      usage(argv[0]);
      exit(1);
    }
  }

  if( options.environment.empty() ) {
    options.environment="development";
    std::cerr << "No environment specified, defaulting to 'development'\n";
  } 
 
  if (options.environment=="staging") {
    options.topic=options.raw?"kvalobs.staging.raw":"kvalobs.staging.checked";
    options.topicLowpri="kvalobs.staging.checked.lowpri";
  } else if (options.environment=="production") {
    options.topic=options.raw?"kvalobs.production.raw":"kvalobs.production.checked";
    options.topicLowpri="kvalobs.production.checked.lowpri";
  } else if( options.environment=="development") {
    options.topic=options.raw?"kvalobs.development.raw":"kvalobs.development.checked";
    options.topicLowpri="kvalobs.development.checked.lowpri";
  } else {
    std::cerr << "Unknown environment '" << options.environment << "'\n";
    exit(1);
  }
 

  if ( options.connections.empty() ){
    options.connections.push_back("user=kvdist dbname=kvqueue host=157.249.78.125 port=5432");
    options.connections.push_back("user=kvdist dbname=kvqueue host=157.249.72.95 port=5432");
    std::cerr << "No connections specified. Using default connections.\n";
    for ( auto &connection : options.connections ) {
      std::cerr << "  -" << connection << '\n';
    }
  }

  if ( options.consumer.empty() ) {
    options.consumer = "kvconsumer";
    std::cerr << "No consumer specified, defaulting to 'kvconsumer'\n";
  }

  return options;
}

void consumeRaw(PgMessaging *queue, Options options) {
  while (!stopRequested) {
    const std::vector<Message> messages = queue->poll(options.consumer, options.topic);
    if (messages.empty()) {
      std::this_thread::sleep_for(std::chrono::seconds(1));
      continue;
    }

    for (const Message &message : messages) {
      std::cout << "\n------ id=" << message.id << " topic=" << message.topic
                << " created_at=" << message.created_at << "-----\n"
                << message.data 
                << "\n------- end message -------\n";
    }
    queue->commit_offset(options.consumer, options.topic, messages.back().id);
    std::cout << "Committed offset " << messages.back().id << '\n';
  }
}

void consumeChecked(PgMessaging *queue, Options options) {
  bool lowPri;
  std::vector<Message> messages;
  while (!stopRequested) {
    messages.clear();
    lowPri=false;
    messages = queue->poll(options.consumer, options.topic);
    if (messages.empty()) {
      lowPri=true;
      messages = queue->poll(options.consumer, options.topicLowpri);
    }
    if (messages.empty()) {
      std::this_thread::sleep_for(std::chrono::seconds(1));
      continue;
    }

    for (const Message &message : messages) {
      std::cout << "\n------ " << (lowPri ? "(lowpri)" : "") << " id=" << message.id << " topic=" << message.topic
                << " created_at=" << message.created_at << "-----\n"
                << message.data 
                << "\n------- end message -------\n";
    }
    if( lowPri) {
      queue->commit_offset(options.consumer, options.topicLowpri, messages.back().id);
    } else {
      queue->commit_offset(options.consumer, options.topic, messages.back().id);
    }
    std::cout << "Committed offset '" << (lowPri ? options.topicLowpri : options.topic) << "' " << messages.back().id << '\n';
  }
}



} // namespace

int main(int argc, char **argv) {
  
  Options options = parse_options(argc, argv);

  if (options.environment.empty() || options.topic.empty() || options.consumer.empty() || options.connections.empty()) {
    usage(argv[0]);
    return 2;
  }
  std::signal(SIGINT, requestStop);

  try {
    PgCluster cluster(options.connections, options.environment, options.consumer);
    std::unique_ptr<PgMessaging> queue = std::make_unique<PgMessaging>(cluster);
    auto topo = cluster.topology();
    std::cerr << "Cluster topology:\n";
    for (const auto &node : topo) {
      std::cerr << "  - " << node.conninfo << "  primary: " << (node.is_primary ? "true" : "false") << '\n';
    }

    if (options.raw) {
        queue->register_consumer(options.consumer, options.topic);
    } else {
        queue->register_consumer(options.consumer, options.topic);
        queue->register_consumer(options.consumer, options.topicLowpri);
    }

    if (options.raw ) {
      std::cout << "Consuming raw data '" << options.topic << "' as " << options.consumer
                << "; press Ctrl-C to stop\n";
    } else {
      std::cout << "Consuming checked data '" << options.topic << "' and '" << options.topicLowpri << "' as " << options.consumer
                << "; press Ctrl-C to stop\n";
    }
    bool reconnect=false;
    while (!stopRequested) {
      if (!cluster.connected()) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
        cluster.reprobe();
        reconnect = true;
        continue;
      }
      try{
        if (reconnect) {
          queue = std::make_unique<PgMessaging>(cluster);
          reconnect = false;
        }
        if( options.raw ) {
          consumeRaw(queue.get(), options);
        } else {
          consumeChecked(queue.get(), options);
        }
      } catch (const std::exception &error) {
        std::cerr << "pgqueue_consumer: " << error.what() << '\n';
      }
    }
    std::cout << "Stopped\n";
  } catch (const std::exception &error) {
    std::cerr << "pgqueue_consumer: " << error.what() << '\n';
    return 1;
  }

  return 0;
}