/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   parsingBody.cpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hboudar <hboudar@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/20 00:16:24 by hboudar           #+#    #+#             */
/*   Updated: 2025/04/06 13:32:07 by hboudar          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../server.hpp"

bool multiPartFormData(client_info &client) {
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
  client.filename = filename;
  client.contentType = contentType;

  client.chunk.erase(0, pos + 2);
  return true;  
}

bool bodyType(client_info& client) {
  if (client.method.empty() == false || !client.headersTaken || client.bodyTypeTaken)
    return true;

  client.isChunked = false;
  std::map<std::string, std::string>::iterator it = client.headers.find("transfer-encoding");
  if (it != client.headers.end() && it->second == "chunked") {
    it = client.headers.find("content-type");
    if (it != client.headers.end()) {
      client.contentType = it->second;
      if (client.contentType.find("multipart/form-data") != std::string::npos) {
        client.boundary = getBoundary(client.contentType);
        if (client.boundary.empty()) {
          std::cerr << "Error: Invalid multipart boundary" << std::endl;
          client.boundary.clear();
          return false; //respond and clear client;
        }
        if (multiPartFormData(client) == false)
          return false; //respond and clear client;
      }
    }
    client.bodyTypeTaken = 1;
    client.isChunked = true;
  } else {
    // NORSML BODY
  }

  std::cerr << "the body type[end]\n" << std::endl;
  return true;
}

bool takeBody_ChunkedFormData(client_info &client) {
  if (client.bodyReached == false || client.chunk.empty() == true)
    return false;
  std::cerr << "taking body[start]" << std::endl;

  // std::ofstream file(client.filename, std::ios::binary | std::ios::app);
  // file << client.chunk;
  // client.chunk.clear();
  // close();

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

  std::cerr << "taking body[end]\n" << std::endl;
  return true;
}