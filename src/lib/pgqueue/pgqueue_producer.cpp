#include "pgqueue.h"

#include <exception>
#include <fstream>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>
#include <getopt.h>

namespace {

std::string read_file(const std::string &path) {
  std::ifstream in(path, std::ios::binary);
  if (!in) {
    throw std::runtime_error("Could not open file: " + path);
  }
  std::ostringstream contents;
  contents << in.rdbuf();
  return contents.str();
}

void usage(const char *program) {
  std::cerr << "Usage: " << program << " [OPTIONS] [CONNECTIONS...]\n";
  std::cerr << "Options:\n";
  std::cerr << "  -e, --environment ENV    Set the environment (default: dev)\n";
  std::cerr << "  -t, --topic TOPIC        Set the topic (default: kvalobs.test.raw)\n";
  std::cerr << "  -m, --message MESSAGE    Set the message to publish\n";
  std::cerr << "  -f, --file FILE          Read the message from a file\n";
  std::cerr << "  -h, --help               Show this help message\n";
  std::cerr << "\n\nIf both -m and -f are specified, -m takes precedence.\n";
}
}  // namespace

struct Options {
  std::string environment;
  std::string topic;
  std::string message;
  std::string file;
  std::vector<std::string> connections;
};


Options parse_options(int argc, char **argv) {
  Options options;
  static struct option long_options[] = {
      {"environment", required_argument, nullptr, 'e'},
      {"topic", required_argument, nullptr, 't'},
      {"message", required_argument, nullptr, 'm'},
      {"file", required_argument, nullptr, 'f'},
      {"help", no_argument, nullptr, 'h'},
      {nullptr, 0, nullptr, 0}
  };
  int option_index = 0;
  int opt;

  while ((opt = getopt_long(argc, argv, "e:t:m:c:hf:", long_options, &option_index)) != -1) {
    switch (opt) {
    case 'e':
      options.environment = optarg;
      break;
    case 't':
      options.topic = optarg;
      break;
    case 'm':
      options.message = optarg;
      break;
    case 'f':
    options.file=optarg;
      // Handle file input for message here if needed
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
    options.connections.push_back("user=kvproc dbname=kvqueue host=157.249.78.125 port=5432");
    options.connections.push_back("user=kvproc dbname=kvqueue host=157.249.72.95 port=5432");
    std::cerr << "No connections specified. Using default connections.\n";
  }

  if (options.file.empty() && options.message.empty()) {
    std::cerr << "No message or file specified.At least one must be provided.\n";
    usage(argv[0]);
    exit(1);
  }

  return options;
}




int main(int argc, char **argv) {
  Options options = parse_options(argc, argv);
  if (options.environment.empty() || options.topic.empty() || 
  (options.message.empty() && options.file.empty()) || options.connections.empty()) {
    usage(argv[0]);
    return 2;
  }

  const std::string environment = options.environment;
  const std::string topic = options.topic;
  std::string message = options.message; 
  const std::vector<std::string> connections = options.connections;

  if (message.empty() ) {
    message = read_file(options.file);
  }

  try {
    PgCluster cluster(connections, environment, "pgqueue_producer");
    PgMessaging queue(cluster);
    queue.create_topic(topic);

    const long long id = queue.publish(topic, message);
    std::cout << "Published message " << id << " to topic " << topic << '\n';
  } catch (const std::exception &error) {
    std::cerr << "pgqueue_producer: " << error.what() << '\n';
    return 1;
  }

  return 0;
}