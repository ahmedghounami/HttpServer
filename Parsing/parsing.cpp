/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   parsing.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hboudar <hboudar@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/09 17:44:02 by hboudar           #+#    #+#             */
/*   Updated: 2025/03/17 15:52:27 by hboudar          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../server.hpp"

bool request_line(client_info &client) {
  if (client.method.empty() == false)
    return true;

  size_t pos = client.chunk.find("\r\n");
  if (pos == std::string::npos)
    return true;
  std::cerr << "request line[start]" << std::endl;

  std::string requestLine = client.chunk.substr(0, pos);
  client.chunk.erase(0, pos + 2);

  size_t firstSP = requestLine.find(' ');
  size_t secondSP = requestLine.find(' ', firstSP + 1);
  size_t thirdSP = requestLine.find(' ', secondSP + 1);

  if (firstSP == std::string::npos || secondSP == std::string::npos ||
      thirdSP != std::string::npos) {
    std::cerr << "Error: Malformed request line (Incorrect spaces)"
              << std::endl;
    //respond and clear client;
    return false;
  }

  client.method = requestLine.substr(0, firstSP);
  client.uri = requestLine.substr(firstSP + 1, secondSP - firstSP - 1);
  client.version = requestLine.substr(secondSP + 1);

  if (client.method != "GET" && client.method != "DELETE" &&
      client.method != "POST") {
    std::cerr << "Error: Unsupported HTTP method: " << client.method
              << std::endl;
    // respond then clear client;
    return false;
  }

  if (client.uri.empty() || client.uri[0] != '/') {
    std::cerr << "Error: Invalid request-target (URI must start with '/')"
    << std::endl;
    // respond then clear client;
    return false;
  }

  if (client.version != "HTTP/1.1") {
    std::cerr << "Error: Unsupported HTTP version: " << client.version
              << std::endl;
    // respond then clear client;
    return false;
  }
  // std::cout << "method ->" << client.method << " uri ->"
  //           << client.uri << " version->" << client.version << std::endl;

  std::cerr << "request line[end]\n" << std::endl;
  return true;
}

bool headers(client_info &client) {
  if (client.headers.empty() == false)
    return true;

  // std::cerr << "headers[start]" << std::endl;
  size_t pos = client.chunk.find("\r\n\r\n");
  if (pos == std::string::npos)
    return true;

  std::string headers = client.chunk.substr(0, pos);
  client.chunk.erase(0, pos + 4);

  size_t startPos = 0;
  std::string lastKey;
  while (startPos < headers.size()) {
    size_t endPos = headers.find("\r\n", startPos);
    if (endPos == std::string::npos)
      endPos = headers.size();

    std::string line = headers.substr(startPos, endPos - startPos);
    startPos = endPos + 2;

    if (line.empty())
      continue;

    if (!lastKey.empty() && (line[0] == ' ' || line[0] == '\t')) {
      client.headers[lastKey] += " " + trim(line);
      continue;
    }

    size_t delimiterPos = line.find(":");
    if (delimiterPos == std::string::npos) {
      // std::cerr << "Error: Malformed header (missing ':'): " << line
      //           << std::endl;
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
    else if (client.headers.find(key) != client.headers.end())
      client.headers[key] += ", " + value;
    else
      client.headers[key] = value;
      
    lastKey = key;
  }

  if (client.headers.find("host") == client.headers.end()) {
    std::cerr << "Error: Missing 'Host' header" << std::endl;
    // respond and clear client;
    return false;
  }

  // std::map<std::string, std::string>::iterator it;
  // for (it = client.headers.begin(); it != client.headers.end(); ++it) {
  //   std::cout << "header-> " << it->first << ": '" << it->second << "'" << std::endl;
  // }
  // std::multimap<std::string, std::string>::iterator itMulti;
  // for (itMulti = client.multiheaders.begin();
  //      itMulti != client.multiheaders.end(); ++itMulti) {
  //   std::cout << "multiheader-> " << itMulti->first << ": '" << itMulti->second << "'" << std::endl;
  // }
  client.file.isChunked = false;
  client.file.contentLength = 0;
  // std::cerr << "headers[end]\n" << std::endl;
  return true;
}


bool bodyType(client_info& client) {
  if (client.file.contentType.empty() == false
    || client.file.isChunked == true)
    // || client.file.contentLength != 0
    return true;

  // std::cerr << "the body type[start]" << std::endl;
  std::map<std::string, std::string>::iterator it = client.headers.find("transfer-encoding");
  if (it != client.headers.end() && it->second == "chunked") {
      client.file.isChunked = true;
      it = client.headers.find("content-type");
      if (it != client.headers.end()) {
        client.file.contentType = it->second;
        if (client.file.contentType.find("multipart/form-data") != std::string::npos) {
          client.boundary = getBoundary(client.file.contentType);
          if (client.boundary.empty()) {
            std::cerr << "Error: Invalid multipart boundary" << std::endl;
            //respond and clear client;
            client.boundary.clear();
            return false;
          }
          // std::cerr << "the body type[end]\n" << std::endl;
          // return true;
        }
      }
      //other types
  }

  // std::cerr << "the body type[end]\n" << std::endl;
  
  return true;
}

bool multiPartFormData(client_info &client) {
  if (client.file.isChunked == false
      || client.boundary.empty() == true
      || (client.file.filename.empty() == false
          && client.file.contentType.empty() == false))
    return true;

  std::string boundaryMarker = client.boundary;
  size_t pos = client.chunk.find(boundaryMarker);
  if (pos == std::string::npos) {
    std::cerr << "Boundary not found." << std::endl;
    return false;
  }
  pos += boundaryMarker.length();

  size_t headerEndPos = client.chunk.find("\r\n\r\n", pos);
  if (headerEndPos == std::string::npos) {
    std::cerr << "Malformed headers in form-data." << std::endl;
    return false;
  }
  std::string headerPart = client.chunk.substr(pos, headerEndPos - pos);
  pos = headerEndPos + 4;

  std::string filename, contentType, contentDisposition;
  size_t dispositionPos = headerPart.find("Content-Disposition:");
  if (dispositionPos != std::string::npos) {
    size_t endLine = headerPart.find("\r\n", dispositionPos);
    contentDisposition = headerPart.substr(dispositionPos + 20, endLine - (dispositionPos + 20));
    size_t filenamePos = contentDisposition.find("filename=\"");
    if (filenamePos != std::string::npos) {
        filename = contentDisposition.substr(filenamePos + 10);
        filename = filename.substr(0, filename.find("\""));
    }
  }

  size_t typePos = headerPart.find("Content-Type:");
  if (typePos != std::string::npos) {
    size_t endLine = headerPart.find("\r\n", typePos);
    contentType = headerPart.substr(typePos + 13, endLine - (typePos + 13));
  }
  client.file.filename = filename;
  client.file.contentType = contentType;
  client.file.bodyReached = true;
  client.file.bodyTaken = false;

  client.chunk.erase(0, pos + 2);
  return true;  
}

bool takeBody_ChunkedFormData(client_info &client) {
  if (client.file.bodyReached == false || client.chunk.empty() == true)
    return false;
  std::cerr << "taking body[start]" << std::endl;

  // std::ofstream file(client.file.filename, std::ios::binary | std::ios::app);
  // file << client.chunk;
  // client.chunk.clear();
  // file.close();

  while (!client.chunk.empty()) {
    //step 1: read chunk size
    size_t pos = client.chunk.find("\r\n");
    if (pos == std::string::npos) {
      std::cerr << "ERROR: Invalid chunked format (no CRLF after size)" << std::endl;
      //respond and clear client;
      return false;
    }

    std::string chunkSizeStr = client.chunk.substr(0, pos);
    client.chunk.erase(0, pos + 2);

    // size_t chunkSize;
    // std::istringstream(chunkSizeStr) >> std::hex >> chunkSize;

    //step 2: check for final chunk
    }
  return true;   
}

//   std::cerr << "taking body[end]\n" << std::endl;
//   return true;
// }

void parse_chunk(client_info &client) {
  //if GET : call lmossiba function
  //else if Post continue;
  if (!request_line(client) || !headers(client))
    return ;
  if (client.method == "GET")
    //call ahmed function;.
  
  if (!bodyType(client) || !multiPartFormData(client)
       || !takeBody_ChunkedFormData(client))
    return;

  (void)client;
}
