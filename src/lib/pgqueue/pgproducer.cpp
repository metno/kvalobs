#include "pgqueue.h"

#include <exception>
#include <filesystem>
#include <fstream>
#include <getopt.h>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>
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
  std::cerr << "  -t, --topic TOPIC        Set the topic (default: "
               "kvalobs.test.raw)\n";
  std::cerr << "  -m, --message MESSAGE    Set the message to publish.\n"
               "     Default content type 'text/plain; charset=utf-8'.\n";
  std::cerr
      << "  -c, --content-type TYPE   Set the content type of the message\n";
  std::cerr << "  -f, --file FILE          Read the message from a file.\n"
               "     Content type will be guessed based on the file extension "
               "     if not specified.\n"
               "        .txt -> text/plain; charset=utf-8\n"
               "        .xml -> application/xml\n"
               "        .json -> application/json\n"
               "        .bufr -> application/bufr\n";
  std::cerr << "  -h, --help               Show this help message\n";
  std::cerr << "\n\nIf both -m and -f are specified, -m takes precedence.\n";
  std::cerr
      << "  -C, --config FILE        Read topic and connections from a "
         "configuration file\n"
         "\nSettings are taken from the command line first, then from the "
         "configuration\nfile (see pgqueue.conf), then the defaults.\n"
         "  Password should be specified in ~/.pgpass.\n";
}
} // namespace

struct Options {
  std::string topic;
  std::string message;
  std::string file;
  std::string content_type;
  std::string configFile;
  std::vector<std::string> connections;
};

std::string guess_content_type(const std::string &file) {
  std::string ext = std::filesystem::path(file).extension();
  if (ext == ".xml") {
    return "application/xml";
  } else if (ext == ".json") {
    return "application/json";
  } else if (ext == ".txt") {
    return "text/plain; charset=utf-8";
  } else if (ext == ".bufr") {
    return "application/bufr";
  }
  return "application/octet-stream";
}

Options parse_options(int argc, char **argv) {
  Options options;
  static struct option long_options[] = {
      {"topic", required_argument, nullptr, 't'},
      {"message", required_argument, nullptr, 'm'},
      {"content-type", required_argument, nullptr, 'c'},
      {"file", required_argument, nullptr, 'f'},
      {"config", required_argument, nullptr, 'C'},
      {"help", no_argument, nullptr, 'h'},
      {nullptr, 0, nullptr, 0}};
  int option_index = 0;
  int opt;

  while ((opt = getopt_long(argc, argv, "t:m:c:hf:C:", long_options,
                            &option_index)) != -1) {
    switch (opt) {
    case 't':
      options.topic = optarg;
      break;
    case 'm':
      options.message = optarg;
      break;
    case 'c':
      options.content_type = optarg;
      break;
    case 'f':
      options.file = optarg;
      // Handle file input for message here if needed
      break;
    case 'C':
      options.configFile = optarg;
      break;
    case 'h':
      usage(argv[0]);
      exit(0);
    default:
      usage(argv[0]);
      exit(1);
    }
  }

  // Handle connections from remaining arguments
  for (int i = optind; i < argc; ++i) {
    options.connections.push_back(argv[i]);
  }

  if (!options.configFile.empty()) {
    try {
      const pgqueue::Config cfg = pgqueue::read_config(options.configFile);
      if (options.topic.empty()) {
        options.topic = cfg.topic;
      }
      if (options.connections.empty()) {
        options.connections = cfg.connections;
      }
    } catch (const std::exception &error) {
      std::cerr << "Failed to read config: " << error.what() << '\n';
      exit(1);
    }
  }

  if (options.topic.empty()) {
    std::cerr << "No topic specified. Using default 'kvalobs.test.raw'\n";
    options.topic = "kvalobs.test.raw";
  }

  if (options.connections.empty()) {
    options.connections.push_back(
        "user=kvproc dbname=pgqueue host=157.249.78.125 port=5432");
    options.connections.push_back(
        "user=kvproc dbname=pgqueue host=157.249.72.95 port=5432");
    std::cerr << "No connections specified. Using default connections.\n";
  }

  if (options.content_type.empty()) {
    options.content_type = "application/octet-stream";
  }

  if (options.content_type.empty() && !options.file.empty()) {
    options.content_type = guess_content_type(options.file);
  }
  if (options.content_type.empty() && !options.message.empty()) {
    options.content_type = "text/plain; charset=utf-8";
  }

  if (options.file.empty() && options.message.empty()) {
    std::cerr
        << "No message or file specified.At least one must be provided.\n";
    usage(argv[0]);
    exit(1);
  }

  return options;
}

int main(int argc, char **argv) {
  Options options = parse_options(argc, argv);
  if (options.topic.empty() ||
      (options.message.empty() && options.file.empty()) ||
      options.connections.empty()) {
    usage(argv[0]);
    return 2;
  }

  if (!options.file.empty() && !std::filesystem::exists(options.file)) {
    std::cerr << "File " << options.file << " does not exist.\n";
    return 1;
  }
  const std::string topic = options.topic;
  std::string message = options.message;
  const std::string content_type = options.content_type;

  if (message.empty()) {
    message = read_file(options.file);
  }

  try {
    pgqueue::PgCluster cluster(options.connections, true, "pgproducer");
    pgqueue::PgMessaging queue(cluster);
    queue.create_topic(topic);

    std::uint64_t id = queue.publish(topic, message, content_type);
    std::cout << "Published message " << id << " to topic " << topic << '\n';
  } catch (const std::exception &error) {
    std::cerr << "pgqueue_producer: " << error.what() << '\n';
    return 1;
  }

  return 0;
}
