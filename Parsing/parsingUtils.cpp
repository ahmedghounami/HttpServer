/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   parsingUtils.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hboudar <hboudar@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/15 17:23:29 by hboudar           #+#    #+#             */
/*   Updated: 2025/03/15 18:19:55 by hboudar          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../server.hpp"

std::string trim(const std::string &str) {
  size_t first = str.find_first_not_of(" \t");
  if (first == std::string::npos)
    return "";
  size_t last = str.find_last_not_of(" \t");
  return str.substr(first, last - first + 1);
}

bool isMultiValueHeader(const std::string &header) {
  static const char *multiHeader[] = {"set-cookie",          "www-authenticate",
                                      "proxy-authenticate",  "authorization",
                                      "proxy-authorization", "warning"};
  for (size_t i = 0; i < sizeof(multiHeader) / sizeof(multiHeader[0]); ++i) {
    if (header == multiHeader[i])
      return true;
  }

  return false;
}

bool isValidHeaderKey(const std::string &key) {
  if (key.empty())
    return false;
  for (size_t i = 0; i < key.length(); ++i) {
    if (!isalpha(key[i]) && key[i] != '-' && key[i] != '_')
      return false;
  }
  return true;
}

bool isValidHeaderValue(const std::string &value) {
  for (size_t i = 0; i < value.length(); ++i) {
    if (iscntrl(value[i]) && value[i] != '\t') {
      return false;
    }
  }
  return true;
}

std::string toLower(const std::string& str) {
    std::string lowerStr = str;
    for (size_t i = 0; i < lowerStr.length(); ++i) {
        lowerStr[i] = std::tolower(lowerStr[i]);
    }
    return lowerStr;
}

std::string getBoundary(const std::string &contentType) {
  size_t pos = contentType.find("boundary=");
  if (pos != std::string::npos)
    return "--" + contentType.substr(pos + 9);
  return "";
}

bool isValidContentLength(const std::string &lengthStr) {
  std::string trlen = trim(lengthStr);
  
  for(size_t i = 0; i < trlen.length(); ++i) {
    if (!std::isdigit(trlen[i]))
      return false;
  }
  return true;
}


/*
  it = client.headers.find("content-type");
  if (it != client.headers.end()) {
    client.contentType = it->second;
    if (client.contentType.find("multipart/form-data") != std::string::npos) {
        std::string boundary = getBoundary(client.contentType);
        if (boundary.empty()) {
          //respond and clear client;
          return false;
        }
        client.boundary = boundary;
        std::cout << "-> form-data: " << client.boundary << " <-" << std::endl;
        return true;
    } else if (client.contentType == "text/html" || client.contentType == "text/plain"
                || client.contentType == "text/javascript" || client.contentType == "application/json"
                || client.contentType == "application/xml") {
      std::cout << "-> raw: " << client.contentType << " <-" << std::endl;
      return true;
    }
 }
  it = client.headers.find("content-length");
  if (it != client.headers.end()) {
  std::string lengthStr = it->second;
  if (isValidContentLength(lengthStr)) { // Convert to long and store
      std::istringstream iss(lengthStr);
      long length;
      iss >> length;
      client.contentLength = length;
      std::cout << "Content-Length: " << client.contentLength << std::endl;
      return true;
  } else {
    // respond and clear client;
    return false;
  }
 }
*/