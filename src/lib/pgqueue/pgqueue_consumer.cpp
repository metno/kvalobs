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
  std::cerr << "  -e, --environment ENV    Set the environment (default: dev)\n";
  std::cerr << "  -t, --topic TOPIC        Set the topic (default: kvalobs.test.raw)\n";
  std::cerr << "  -h, --help               Show this help message\n";
  std::cerr << "\n\nIf both -m and -f are specified, -m takes precedence.\n";
}

struct Options {
  std::string environment;
  std::string topic;
  std::string consumer;
  std::vector<std::string> connections;
};


Options parse_options(int argc, char **argv) {
  Options options;
  static struct option long_options[] = {
      {"environment", required_argument, nullptr, 'e'},
      {"topic", required_argument, nullptr, 't'},
      {"consumer", required_argument, nullptr, 'c'},
      {"help", no_argument, nullptr, 'h'},
      {nullptr, 0, nullptr, 0}
  };
  int option_index = 0;
  int opt;

  while ((opt = getopt_long(argc, argv, "e:t:c:h", long_options, &option_index)) != -1) {
    switch (opt) {
    case 'e':
      options.environment = optarg;
      break;
    case 't':
      options.topic = optarg;
      break;
    case 'c':
      options.consumer = optarg;
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
    options.environment="dev";
    std::cerr << "No environment specified, defaulting to 'dev'\n";
  } 
  if ( options.topic.empty() ){
    std::cerr << "No topic specified. Using default 'kvalobs.test.raw'\n";
    options.topic = "kvalobs.test.raw";
  }

  // Handle connections from remaining arguments
  for (int i = optind; i < argc; ++i) {
    options.connections.push_back(argv[i]);
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

} // namespace

int main(int argc, char **argv) {
  
  Options options = parse_options(argc, argv);

  if (options.environment.empty() || options.topic.empty() || options.consumer.empty() || options.connections.empty()) {
    usage(argv[0]);
    return 2;
  }
  const std::string environment = options.environment;
  const std::string topic = options.topic;
  const std::vector<std::string> connections = options.connections;

  std::signal(SIGINT, requestStop);

  try {
    PgCluster cluster(connections, environment, options.consumer);
    PgMessaging queue(cluster);

    auto topo = cluster.topology();
    std::cerr << "Cluster topology:\n";
    for (const auto &node : topo) {
      std::cerr << "  - " << node.conninfo << "  primary: " << (node.is_primary ? "true" : "false") << '\n';
    }
    queue.register_consumer(options.consumer, topic);

    std::cout << "Consuming " << topic << " as " << options.consumer
              << "; press Ctrl-C to stop\n";
    while (!stopRequested) {
      const std::vector<Message> messages = queue.poll(options.consumer, topic);
      if (messages.empty()) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
        continue;
      }

      for (const Message &message : messages) {
        std::cout << "id=" << message.id << " topic=" << message.topic
                  << " created_at=" << message.created_at << '\n'
                  << message.data << '\n';
      }

      queue.commit_offset(options.consumer, topic, messages.back().id);
      std::cout << "Committed offset " << messages.back().id << '\n';
    }

    std::cout << "Stopped\n";
  } catch (const std::exception &error) {
    std::cerr << "pgqueue_consumer: " << error.what() << '\n';
    return 1;
  }

  return 0;
}