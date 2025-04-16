/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   parsingBody.cpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hboudar <hboudar@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/20 00:16:24 by hboudar           #+#    #+#             */
/*   Updated: 2025/04/16 21:51:33 by hboudar          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../server.hpp"

void ChunkedData(client_info& client) {
  while (!client.data.empty() || (client.isChunked && !client.bodyTaken)) {
    std::cerr << "the loop is looping" << std::endl;
    size_t pos = client.data.find("\r\n");
    client.pos = client.data.find(client.boundary);
    if (client.pos == pos + 2)   {//found boundary after chunk size
      client.data = client.data.substr(pos + 2);
      client.pos = 0;
      NewFile(client);
      client.pos = client.data.find("\r\n");
      std::string ChunkSizeString = client.data.substr(0, client.pos);
      client.data = client.data.substr(client.pos + 2);
      std::istringstream iss(ChunkSizeString);
      client.chunkSize = 0;
      iss >> std::hex >> client.chunkSize;
      if (client.chunkSize + 2 > client.data.size()) {
        client.bytesLeft = client.chunkSize - (client.data.size() - 2);
        client.chunkData = client.data.substr(0, client.data.size() - 2);
        client.data.clear();
      } else {
        client.chunkData = client.data.substr(0, client.chunkSize);
        client.data = client.data.substr(client.chunkSize + 2);
        client.bytesLeft = 0;
      }
      if (!client.chunkData.empty())
        writeToFile(client.chunkData, client.file_fd);
    } else {
      if (client.bytesLeft > 0 && client.data.size() >= client.bytesLeft) {
        client.chunkData = client.data.substr(0, client.bytesLeft);
        if (!client.chunkData.empty())
          writeToFile(client.chunkData, client.file_fd);
        client.data = client.data.substr(client.bytesLeft);
        client.bytesLeft = 0;
      }
      else if (client.bytesLeft > 0) {
        client.chunkData = client.data;
        if (!client.chunkData.empty())
          writeToFile(client.chunkData, client.file_fd);

        client.bytesLeft -= client.chunkData.size();
        client.data.clear();
        break ;
      }
    }
    if (client.data.empty()) {
      std::cerr << "Breaking out of the loop : data is empty" << std::endl;
      break ;
    }
    if ((client.data.size() == 2 && client.data == "\r\n")
        || (client.data.size() == 4 && client.data == "\r\n\r\n")
        || (client.data.size() == 6 && client.data == "\r\n\r\n\r\n")) {
      std::cerr << "Only CRLF left" << std::endl;
      break ;
    }
    exit (1);
  }

  size_t pos = client.data.find(client.boundary + "--");
  if (pos != std::string::npos) {
    std::cerr << "Found end boundary and set bodyTaken" << std::endl;
    client.bodyTaken = true;
    std::cerr << "Data: |" << client.data << "|" << std::endl;
    exit (1);
  }
}

bool takeBodyType(client_info& client) {
  if (client.method.empty() || !client.headersTaken || client.bodyTypeTaken)
    return true;

  client.isChunked = false;
  client.bodyTaken = false;
  client.bytesLeft = 0;
  client.chunkData = "";

  // client.file_fd = open("file", O_WRONLY | O_CREAT | O_TRUNC, 0644);
  // if (client.file_fd == -1) {
  //   std::cerr << "Error: Failed to open file" << std::endl;
  //   exit (1);
  // }

  std::map<std::string, std::string>::iterator it = client.headers.find("transfer-encoding");
  if (it != client.headers.end() && it->second == "chunked") {
    client.isChunked = true;
    it = client.headers.find("content-type");
    if (it != client.headers.end()) {
      client.ContentType = it->second;
      if (client.ContentType.find("multipart/form-data") != std::string::npos) {
        client.boundary = getBoundary(client.ContentType);
        if (client.boundary.empty()) {
          std::cerr << "Error: Invalid multipart boundary" << std::endl;
          client.boundary.clear();
          return false; //respond and clear client;
        }
        // formDataChunked(client);
        client.bodyTypeTaken = 1;
        client.boundary = "--" + client.boundary;
        // size_t pos = client.data.find("\r\n");
        // if (pos != std::string::npos)
        //   client.data = client.data.substr(pos + 2);
      } else {
        // otherDataChunked(client);
        client.bodyTypeTaken = 2;
      }
    } else {
      std::cerr << "Error: Missing 'Content-Type' header" << std::endl;
      exit (1);
    }
  } else {
    it = client.headers.find("content-type");
    if (it != client.headers.end()) {
      client.ContentType = it->second;
      if (client.ContentType.find("multipart/form-data") != std::string::npos) {
        client.boundary = getBoundary(client.ContentType);
        if (client.boundary.empty()) {
          std::cerr << "Error: Invalid multipart boundary" << std::endl;
          client.boundary.clear();
          exit (1);
        }
        // formData(client);
        client.bodyTypeTaken = 3;

      } else {
        // otherData(client);
        client.bodyTypeTaken = 4;
      }
    } else { // No content-type header
      std::cerr << "Error: Missing 'Content-Type' header" << std::endl;
      return false; //respond and clear client;
    }
  }
  return true;
}
