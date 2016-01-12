/*
 wdb - weather and water data storage

 Copyright (C) 2007 met.no

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


#include <fstream>
#include <iostream>
#include <stdexcept>
#include <mutex>  // NOLINT(build/c++11)
#include "lib/miutil/httpclient.h"


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


size_t write_function(void *ptr, size_t size, size_t nmemb, void *stream);

void throwExceptionOnError(CURLcode code, const std::string &msg = "") {
  if (code != CURLE_OK) {
    string message = msg;
    if (msg.empty())
      message = curl_easy_strerror(code);

    switch (code) {
      case CURLE_URL_MALFORMAT:
        throw HttpUrlError(message);
      case CURLE_COULDNT_RESOLVE_PROXY:
      case CURLE_COULDNT_RESOLVE_HOST:
      case CURLE_COULDNT_CONNECT:
        throw HttpConnectError(message);
      case CURLE_REMOTE_ACCESS_DENIED:
      case CURLE_LOGIN_DENIED:
        throw HttpAccessDenied(message);
      case CURLE_WRITE_ERROR:
      case CURLE_READ_ERROR:
      case CURLE_SEND_ERROR:
      case CURLE_RECV_ERROR:
        throw HttpIOError(message);
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
        throw HttpSSLError(message);
      case CURLE_OPERATION_TIMEDOUT:
        throw HttpTimeout(message);
      default:
        throw HttpException(message);
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

}  // namespace

namespace miutil {

HTTPClient::HTTPClient()
    : curl(0),
      out(0) {
  errbuf = new char[CURL_ERROR_SIZE];
  std::call_once(initCurl, curl_global_init, CURL_GLOBAL_ALL);
  open();
}

HTTPClient::~HTTPClient() {
  close();
  delete[] errbuf;
}

void HTTPClient::open() {
  CURLcode status;
  close();

  curl = curl_easy_init();

  if (curl) {
    errbuf[0] = '\0';
    if ((status = curl_easy_setopt(curl, CURLOPT_ERRORBUFFER, errbuf))
        != CURLE_OK) {
      close();
      throw HttpInitError();
    }
    if ((status = curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_function))
        != CURLE_OK) {
      close();
      throwExceptionOnError(status);
    }

    if ((status = curl_easy_setopt(curl, CURLOPT_WRITEDATA, this))
        != CURLE_OK) {
      close();
      throwExceptionOnError(status);
    }
  }

  if (curl == 0)
    throw HttpInitError();
}

void HTTPClient::close() {
  if (curl) {
    curl_easy_cleanup(curl);
    curl = 0;
  }
}

void HTTPClient::get(const std::string &url, std::ostream &content) {
  CURLcode ret;

  if (!curl)
    throw HttpInitError();

  errbuf[0] = '\0';
  if ((ret = curl_easy_setopt(curl, CURLOPT_HTTPGET, 1)) != CURLE_OK) {
    close();
    throwExceptionOnError(ret);
  }

  ret = curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
  throwExceptionOnError(ret);

  out = &content;
  ret = curl_easy_perform(curl);
  out = 0;

  throwExceptionOnError(ret);
}

void HTTPClient::post(const std::string &url, const std::string &content,
                      const std::string &contentType) {
  content_.str("");
  CURLcode ret;
  ostringstream item;
  struct curl_slist *header = NULL;

  if (!curl)
    throw HttpInitError();

  errbuf[0] = '\0';
  item << "Content-Type: " << contentType;
  header = curl_slist_append(header, item.str().c_str());
  if ((ret = curl_easy_setopt(curl, CURLOPT_HTTPHEADER, header)) != CURLE_OK) {
    curl_slist_free_all(header);
    close();
    throwExceptionOnError(ret);
  }

  if ((ret = curl_easy_setopt(curl, CURLOPT_POSTFIELDS, content.c_str()))
      != CURLE_OK) {
    curl_slist_free_all(header);
    close();
    throwExceptionOnError(ret);
  }

  if ((ret = curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, content.length()))
      != CURLE_OK) {
    curl_slist_free_all(header);
    close();
    throwExceptionOnError(ret);
  }

  ret = curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
  throwExceptionOnError(ret);

  out = &content_;
  ret = curl_easy_perform(curl);
  out = 0;
  throwExceptionOnError(ret);
}

void HTTPClient::postFile(const std::string &url, const std::string &file,
                          const std::string &contentType) {
  string data;

  data = readFile(file);
  post(url, data, contentType);
}

long HTTPClient::contenlLength() const {
  double d;

  if (curl) {
    errbuf[0] = '\0';
    if ( curl_easy_getinfo(curl, CURLINFO_CONTENT_LENGTH_DOWNLOAD, &d)
        == CURLE_OK)
      return static_cast<long>(d);
  }

  return -1;
}

std::string HTTPClient::contentType() const {
  char *ct;

  if (curl) {
    errbuf[0] = '\0';
    if ( curl_easy_getinfo(curl, CURLINFO_CONTENT_TYPE, &ct)
        == CURLE_OK && ct != NULL)
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

}  // namespace miutil

namespace {
size_t write_function(void *ptr, size_t size, size_t nmemb, void *stream) {
  size_t n = 0;
  int len = size * nmemb;

  if (stream != NULL) {
    miutil::HTTPClient *client = static_cast<miutil::HTTPClient*>(stream);
    std::ostream *out = client->outStream();

    if (out) {
      out->write(static_cast<char*>(ptr), len);
      n = len;
    }
  }

  return n;
}

}  // namespace
