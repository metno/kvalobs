/*
 kvalobs

 Copyright (C) 2016 met.no

 Contact information:
 Norwegian Meteorological Institute
 Box 43 Blindern
 0313 OSLO
 NORWAY
 E-mail: wdb@met.no

 This program is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 2 of the License, or
 (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program; if not, write to the Free Software
 Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 MA  02110-1301, USA
 */

#include "lib/miutil/httpclient.h"
#include <curl/curl.h>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <sstream>
#include <mutex>

using std::fstream;
using std::string;
using std::endl;
using std::ostringstream;
using miutil::HttpUrlError;
using miutil::HttpConnectError;
using miutil::HttpAccessDenied;
using miutil::HttpIOError;
using miutil::HttpSSLError;
using miutil::HttpTimeout;
using miutil::HttpException;

namespace {
std::once_flag initCurl;
size_t write_function(char *ptr, size_t size, size_t nmemb, void *stream);

void throwExceptionOnError(CURLcode code, const std::string &msg = "") {
  if (code != CURLE_OK) {
    ostringstream message;
    if (!msg.empty())
      message << msg << " CURL err: ";

    message << curl_easy_strerror(code);

    switch (code) {
      case CURLE_URL_MALFORMAT:
        throw HttpUrlError(message.str());
      case CURLE_COULDNT_RESOLVE_PROXY:
      case CURLE_COULDNT_RESOLVE_HOST:
      case CURLE_COULDNT_CONNECT:
        throw HttpConnectError(message.str());
      case CURLE_REMOTE_ACCESS_DENIED:
      case CURLE_LOGIN_DENIED:
        throw HttpAccessDenied(message.str());
      case CURLE_WRITE_ERROR:
      case CURLE_READ_ERROR:
      case CURLE_SEND_ERROR:
      case CURLE_RECV_ERROR:
        throw HttpIOError(message.str());
      case CURLE_SSL_CONNECT_ERROR:
      case CURLE_SSL_ENGINE_NOTFOUND:
      case CURLE_SSL_ENGINE_SETFAILED:
      case CURLE_SSL_CERTPROBLEM:
      case CURLE_SSL_CIPHER:
      case CURLE_SSL_CACERT:
      case CURLE_SSL_ENGINE_INITFAILED:
      case CURLE_SSL_CACERT_BADFILE:
      case CURLE_SSL_SHUTDOWN_FAILED:
      case CURLE_SSL_CRL_BADFILE:
      case CURLE_SSL_ISSUER_ERROR:
      case CURLE_USE_SSL_FAILED:
        throw HttpSSLError(message.str());
      case CURLE_OPERATION_TIMEDOUT:
        throw HttpTimeout(message.str());
      default:
        throw HttpException(message.str());
    }
  }
}

string readFile(const std::string &file) {
  fstream fist(file.c_str());
  string line;
  ostringstream ost;

  if (!fist)
    throw miutil::HttpIOError("Cant open file: " + file + ".");

  while (getline(fist, line))
    ost << line << "\n";

  if (!fist.eof()) {
    fist.close();
    throw miutil::HttpIOError("Cant read file: " + file + ".");
  }

  fist.close();
  return ost.str();
}

class Closer {
  miutil::HTTPClient *client;
 public:
  explicit Closer(miutil::HTTPClient *client_)
      : client(client_) {
  }

  ~Closer() {
    if (client)
      client->close();
  }

  void release() {
    client = nullptr;
  }
};

}  // namespace

namespace miutil {

HTTPClient::HTTPClient()
    : curl(nullptr),
      header(nullptr),
      out(nullptr) {
  errbuf = new char[CURL_ERROR_SIZE];
  std::call_once(initCurl, curl_global_init, CURL_GLOBAL_ALL);
}

HTTPClient::~HTTPClient() {
  close();
  delete[] errbuf;
}

void HTTPClient::open() {
  CURLcode status;
  Closer closer(this);

  curl = curl_easy_init();

  if (curl) {
    throwExceptionOnError(curl_easy_setopt(curl, CURLOPT_ERRORBUFFER, errbuf), "Failed to set error buffer.");
    throwExceptionOnError(curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_function), "Failed to set the 'write_function'.");
    throwExceptionOnError(curl_easy_setopt(curl, CURLOPT_WRITEDATA, this), "Failed to set the extra data (a pointer to HTTPClient).");
  }

  if (!curl)
    throw HttpInitError();
  closer.release();
}

void HTTPClient::close() {
  if (curl) {
    curl_easy_cleanup(curl);
    curl_slist_free_all(header);
    header = nullptr;
    curl = nullptr;
  }
}

void HTTPClient::get(const std::string &url, std::ostream &content) {
  CURLcode ret;
  Closer closer(this);
  open();

  if (!curl)
    throw HttpInitError();

  throwExceptionOnError(curl_easy_setopt(curl, CURLOPT_HTTPGET, 1), "HTTP GET.");
  throwExceptionOnError(curl_easy_setopt(curl, CURLOPT_URL, url.c_str()), "GET: Failed to set url '" + url + "'.");

  out = &content;
  ret = curl_easy_perform(curl);
  out = nullptr;

  throwExceptionOnError(ret, "Do GET.");
  closer.release();
}

void HTTPClient::post(const std::string &url, const std::string &content, const std::string &contentType) {
  content_.str("");
  CURLcode ret;
  ostringstream item;
  Closer closer(this);

  open();

  if (!curl)
    throw HttpInitError();

  item << "Content-Type: " << contentType;
  header = curl_slist_append(header, item.str().c_str());

  throwExceptionOnError(curl_easy_setopt(curl, CURLOPT_HTTPHEADER, header), "POST: Failed to set headers.");
  throwExceptionOnError(curl_easy_setopt(curl, CURLOPT_POSTFIELDS, content.c_str()), "POST: Failed to set content.");
  throwExceptionOnError(curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, content.length()), "POST: Failed to set content size.");
  throwExceptionOnError(curl_easy_setopt(curl, CURLOPT_URL, url.c_str()), "POST: Failed to set url '" + url + "'.");

  out = &content_;
  ret = curl_easy_perform(curl);
  out = nullptr;
  throwExceptionOnError(ret, "Do POST.");
  closer.release();
}

void HTTPClient::postFile(const std::string &url, const std::string &file, const std::string &contentType) {
  string data;

  data = readFile(file);
  post(url, data, contentType);
}

long HTTPClient::contenlLength() const {
  double d;

  if (curl) {
    errbuf[0] = '\0';
    if ( curl_easy_getinfo(curl, CURLINFO_CONTENT_LENGTH_DOWNLOAD, &d) == CURLE_OK)
      return static_cast<long>(d);
  }

  return -1;
}

std::string HTTPClient::contentType() const {
  char *ct;

  if (curl) {
    errbuf[0] = '\0';
    if ( curl_easy_getinfo(curl, CURLINFO_CONTENT_TYPE, &ct) == CURLE_OK && ct != NULL)
      return string(ct);
  }

  return string();
}

long HTTPClient::returnCode() const {
  long rc;

  if (curl) {
    errbuf[0] = '\0';
    if ( curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &rc) == CURLE_OK)
      return rc;
  }

  return -1;
}

double HTTPClient::totalTime() const {
  double dt = -1;

  if (curl) {
    errbuf[0] = '\0';
    if ( curl_easy_getinfo(curl, CURLINFO_TOTAL_TIME, &dt) == CURLE_OK)
      return dt;
  }

  return dt;
}

void HTTPClient::log(const std::string &msg) {
  std::clog << msg << std::endl;
}

}  // namespace miutil

namespace {
size_t write_function(char *ptr, size_t size, size_t nmemb, void *stream) {
  size_t n = 0;
  size_t len = size * nmemb;

  if (len == 0)
    return 0;

  if (stream != NULL) {
    miutil::HTTPClient *client = static_cast<miutil::HTTPClient*>(stream);
    std::ostream *out = client->outStream();

    if (out) {
      out->write(ptr, len);
      n = len;
    } else {
      client->log("HTTPClient: write_function: No pointer to a stream given.");
    }
  } else {
    std::clog << "HTTPClient: write_function: No pointer to a HTTPClient given." << std::endl;
  }

  return n;
}

}  // namespace
