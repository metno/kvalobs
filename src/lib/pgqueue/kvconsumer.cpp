#include "pgqueue.h"

#include <chrono>
#include <csignal>
#include <cstdlib>
#include <cstring>
#include <exception>
#include <fstream>
#include <getopt.h>
#include <iostream>
#include <pwd.h>
#include <string>
#include <thread>
#include <unistd.h>
#include <vector>

namespace {
volatile std::sig_atomic_t stopRequested = 0;

bool deleteLogAtStart = true;
void requestStop(int) { stopRequested = 1; }

void writeErrorToFile(const std::string &message) {
  if (deleteLogAtStart) {
    std::ofstream errorFile("kvconsumer_error.log", std::ios::trunc);
    deleteLogAtStart = false;
  }

  // Create timestamp
  auto now = std::chrono::system_clock::now();
  std::time_t now_c = std::chrono::system_clock::to_time_t(now);
  std::string timestamp = std::ctime(&now_c);
  timestamp.pop_back(); // remove trailing newline
  std::ofstream errorFile("kvconsumer_error.log", std::ios::app);
  if (errorFile) {
    errorFile << "[" << timestamp << "] " << message << '\n';
  }
}

// Consumer name on the form kvconsumer-cpp-<hostname>-<username>.
std::string defaultConsumerName() {
  char host[256] = {0};
  if (gethostname(host, sizeof(host) - 1) != 0 || host[0] == '\0') {
    std::strcpy(host, "unknown");
  }

  std::string user;
  if (const passwd *pw = getpwuid(geteuid())) {
    user = pw->pw_name;
  } else if (const char *env = std::getenv("USER")) {
    user = env;
  }
  if (user.empty()) {
    user = "unknown";
  }

  return std::string("kvconsumer-cpp-") + host + "-" + user;
}

void usage(const char *program) {
  std::cerr << "Usage: " << program << " [OPTIONS]\n";
  std::cerr << "Options:\n";
  std::cerr << "  -f, --config FILE      Specify a config file. (TODO: "
               "Implement usage)\n";
  std::cerr << "  -d, --domain DOMAIN    Set the domain (default: staging)\n";
  std::cerr << "  -r, --raw                Konsumes the raw queue\n";
  std::cerr << "  -c, --consumer CONSUMER  Set the consumer (default: "
               "kvconsumer-cpp-HOSTNAME-USERNAME)\n";
  std::cerr << "  -h, --help               Show this help message\n\n";
}

struct Options {
  std::string domain;
  std::string topic;
  std::string topicLowpri;
  std::string consumer;
  std::string configFile;
  bool raw;
  std::vector<std::string> connections;
};

Options parse_options(int argc, char **argv) {
  Options options;
  static struct option long_options[] = {
      {"config", required_argument, nullptr, 'f'},
      {"domain", required_argument, nullptr, 'd'},
      {"consumer", required_argument, nullptr, 'c'},
      {"raw", no_argument, nullptr, 'r'},
      {"help", no_argument, nullptr, 'h'},
      {nullptr, 0, nullptr, 0}};
  int option_index = 0;
  int opt;
  options.raw = false;
  while ((opt = getopt_long(argc, argv, "c:h:rd:f:", long_options,
                            &option_index)) != -1) {
    switch (opt) {
    case 'f':
      // Use a config file if specified
      // TODO: Implement reading and using the config file
      std::cerr << "TODO: Config file specified: " << optarg << '\n';
      options.configFile = optarg;
      break;
    case 'c':
      options.consumer = optarg;
      break;
    case 'r':
      options.raw = true;
      break;
    case 'd':
      // Handle domain option if needed
      options.domain = optarg;
      break;
    case 'h':
      usage(argv[0]);
      exit(0);
    default:
      usage(argv[0]);
      exit(1);
    }
  }

  if (options.domain.empty()) {
    options.domain = "staging";
  }

  if (options.topic.empty()) {
    options.topic = options.raw ? "kvalobs." + options.domain + ".raw"
                                : "kvalobs." + options.domain + ".checked";
    options.topicLowpri = "kvalobs." + options.domain + ".checked.lowpri";
  }

  if (options.connections.empty()) {
    options.connections.push_back(
        "user=kvdist dbname=pgqueue host=157.249.78.125 port=5432");
    options.connections.push_back(
        "user=kvdist dbname=pgqueue host=157.249.72.95 port=5432");
    std::cerr << "No connections specified. Using default connections.\n";
    for (auto &connection : options.connections) {
      std::cerr << "  - " << connection << '\n';
    }
  }

  if (options.consumer.empty()) {
    options.consumer = defaultConsumerName();
    std::cerr << "No consumer specified, defaulting to '" << options.consumer
              << "'\n";
  }

  return options;
}

void consumeRaw(pgqueue::PgMessaging *queue, Options options) {
  while (!stopRequested) {
    const std::vector<pgqueue::Message> messages =
        queue->poll(options.consumer, options.topic);
    if (messages.empty()) {
      std::this_thread::sleep_for(std::chrono::seconds(1));
      continue;
    }

    for (const pgqueue::Message &message : messages) {
      std::cout << message;
    }
    queue->commit_offset(options.consumer, options.topic, messages.back().id);
    std::cout << "Committed offset " << messages.back().id << '\n';
  }
}

void consumeChecked(pgqueue::PgMessaging *queue, Options options) {
  bool lowPri;
  std::vector<pgqueue::Message> messages;
  while (!stopRequested) {
    messages.clear();
    lowPri = false;
    messages = queue->poll(options.consumer, options.topic);
    if (messages.empty()) {
      lowPri = true;
      messages = queue->poll(options.consumer, options.topicLowpri);
    }
    if (messages.empty()) {
      std::this_thread::sleep_for(std::chrono::seconds(1));
      continue;
    }

    for (const pgqueue::Message &message : messages) {
      std::cout << message;
    }
    if (lowPri) {
      queue->commit_offset(options.consumer, options.topicLowpri,
                           messages.back().id);
    } else {
      queue->commit_offset(options.consumer, options.topic, messages.back().id);
    }
    std::cout << "Committed offset '"
              << (lowPri ? options.topicLowpri : options.topic) << "' "
              << messages.back().id << '\n';
  }
}

} // namespace

int main(int argc, char **argv) {

  Options options = parse_options(argc, argv);

  std::signal(SIGINT, requestStop);

  try {
    pgqueue::PgCluster cluster(options.connections, options.consumer);
    std::unique_ptr<pgqueue::PgMessaging> queue =
        std::make_unique<pgqueue::PgMessaging>(cluster);
    auto topo = cluster.topology();
    std::cerr << "Cluster topology:\n";
    for (const auto &node : topo) {
      std::cerr << "  - " << node.conninfo
                << "  primary: " << (node.is_primary ? "true" : "false")
                << '\n';
    }

    if (options.raw) {
      queue->register_consumer(options.consumer, options.topic);
    } else {
      queue->register_consumer(options.consumer, options.topic);
      queue->register_consumer(options.consumer, options.topicLowpri);
    }

    if (options.raw) {
      std::cout << "Consuming raw data '" << options.topic << "' as "
                << options.consumer << "; press Ctrl-C to stop\n";
    } else {
      std::cout << "Consuming checked data '" << options.topic << "' and '"
                << options.topicLowpri << "' as " << options.consumer
                << "; press Ctrl-C to stop\n";
    }
    bool reconnect = false;
    while (!stopRequested) {
      if (!cluster.connected()) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
        cluster.reprobe();
        reconnect = true;
        continue;
      }
      try {
        if (reconnect) {
          queue = std::make_unique<pgqueue::PgMessaging>(cluster);
          reconnect = false;
        }
        if (options.raw) {
          consumeRaw(queue.get(), options);
        } else {
          consumeChecked(queue.get(), options);
        }
      } catch (const std::exception &error) {
        std::cerr << "pgqueue_consumer: " << error.what() << '\n';
        writeErrorToFile(std::string("pgqueue_consumer: ") + error.what());
        reconnect = true;
      }
    }
    std::cout << "Stopped\n";
  } catch (const std::exception &error) {

    std::cerr << "pgqueue_consumer: " << error.what() << '\n';
    writeErrorToFile(std::string("pgqueue_consumer: ") + error.what());
    return 1;
  }

  return 0;
}