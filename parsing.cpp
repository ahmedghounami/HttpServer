/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   parsing.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hboudar <hboudar@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/09 17:44:02 by hboudar           #+#    #+#             */
/*   Updated: 2025/03/12 15:26:53 by hboudar          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "server.hpp"

bool request_line(client_info &client) {
  if (!client.method.empty())
    return true;
  size_t pos = client.chunk.find("\r\n");
  if (pos == std::string::npos)
    return true;

  std::string requestLine = client.chunk.substr(0, pos);
  client.chunk.erase(0, pos + 2);

  size_t firstSP = requestLine.find(' ');
  size_t secondSP = requestLine.find(' ', firstSP + 1);
  size_t thirdSP = requestLine.find(' ', secondSP + 1);

  if (firstSP == std::string::npos || secondSP == std::string::npos ||
      thirdSP != std::string::npos) {
    // std::cerr << "Error: Malformed request line (Incorrect spaces)"
    //           << std::endl;
    // respond then clear client;
    return false;
  }

  client.method = requestLine.substr(0, firstSP);
  client.uri = requestLine.substr(firstSP + 1, secondSP - firstSP - 1);
  client.version = requestLine.substr(secondSP + 1);

  if (client.method != "GET" && client.method != "DELETE" &&
      client.method != "POST") {
    // std::cerr << "Error: Unsupported HTTP method: " << client.method
    //           << std::endl;
    // respond then clear client;
    return false;
  }

  if (client.uri.empty() || client.uri[0] != '/') {
    // std::cerr << "Error: Invalid request-target (URI must start with '/')"
    // << std::endl;
    // respond then clear client;
    return false;
  }

  if (client.version != "HTTP/1.1") {
    // std::cerr << "Error: Unsupported HTTP version: " << client.version
    //           << std::endl;
    // respond then clear client;
    return false;
  }
  std::cout << "method ->" << client.method << " uri ->"
            << client.uri << " version->" << client.version << std::endl;

  return true;
}

bool pars_headers(client_info &client) {
  if (!client.headers.empty())
    return true;
  size_t pos = client.chunk.find("\r\n\r\n");
  if (pos == std::string::npos)
    return true;

  std::string headers = client.chunk.substr(0, pos);
  client.chunk.erase(0, pos + 4);

  size_t startPos = 0;
  while (startPos < headers.size()) {
    size_t endPos = headers.find("\r\n", startPos);
    if (endPos == std::string::npos)
      break;

    std::string line = headers.substr(startPos, endPos - startPos);
    startPos = endPos + 2;

    if (line.empty())
      continue;

    size_t delimiterPos = line.find(":");
    if (delimiterPos == std::string::npos) {
      std::cerr << "Error: Malformed header (missing ':'): " << line
                << std::endl;
      // respond and clear client;
      return false;
    }

    std::string key = trim(line.substr(0, delimiterPos));
    std::string value = trim(line.substr(delimiterPos + 1));

    if (key.empty() || value.empty()) {
      std::cerr << "Error: Empty header name or value" << std::endl;
      // respond and clear client;
      return false;
    }

    std::transform(key.begin(), key.end(), key.begin(), ::tolower);
    if (!isValidHeaderKey(key)) {
      std::cerr << "Error: Invalid header name: " << key << std::endl;
      // respond and clear client;
      return false;
    }
    if (!isValidHeaderValue(value)) {
      std::cerr << "Error: Invalid header value: " << value << std::endl;
      // respond and clear client;
      return false;
    }
    if (isMultiValueHeader(key))
      client.multiheaders.insert(std::make_pair(key, value));
    // else if (client.headers.find(key) != client.headers.end())
    //   client.headers[key] += ", " + value;
    else
      client.headers[key] = value;
  }

  if (client.headers.find("host") == client.headers.end()) {
    std::cerr << "Error: Missing 'Host' header" << std::endl;
    // respond and clear client;
    return false;
  }

  std::map<std::string, std::string>::iterator it;
  for (it = client.headers.begin(); it != client.headers.end(); ++it) {
    std::cout << "header-> " << it->first << ": " << it->second << std::endl;
  }
  std::multimap<std::string, std::string>::iterator itMulti;
  for (itMulti = client.multiheaders.begin();
       itMulti != client.multiheaders.end(); ++itMulti) {
    std::cout << "multiheader-> " << itMulti->first << ": " << itMulti->second
              << std::endl;
  }
  client.isChunked = false;
  client.contentLength = 0;
  return true;
}


bool detectBodyType(client_info& client) {
  if (!client.contentType.empty() || client.isChunked != false || client.contentLength != 0) {
    return true;
  }

  std::map<std::string, std::string>::iterator it = client.headers.find("content-type");
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
    } else {
        std::cerr << "Error: Invalid Content-Length value (non-numeric characters found)\n";
        return false;
      }
    }
  //form-data : nothing
  return true;
}
