/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   parsingBody.cpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hboudar <hboudar@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/20 00:16:24 by hboudar           #+#    #+#             */
/*   Updated: 2025/04/17 23:04:50 by hboudar          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../server.hpp"

void ChunkedData(client_info& client) {
  while (!client.data.empty() && client.isChunked && !client.bodyTaken) {


    //here we read header info {name, filename, content-type}
    client.pos = client.data.find(client.boundary + "\r\n");
    if (client.pos != std::string::npos && client.ReadSize == true) {
      client.data = client.data.substr(client.pos);
      NewFile(client);
      client.ReadSize = true;
      if (client.data.empty())
        break ;
    }

    //here we read size {if we don't have it yet}
    if (client.ReadSize ==  true) {
      client.ReadSize = false;
      client.pos = client.data.find("\r\n");
      std::string ChunkSizeString = client.data.substr(0, client.pos);
      client.data = client.data.substr(client.pos + 2);
      std::istringstream iss(ChunkSizeString);
      client.chunkSize = 0;
      iss >> std::hex >> client.chunkSize;
    }


    //here we read data if we have it if not we break
    if (client.chunkSize > 0) {
      if (client.chunkSize + 2 > client.data.size()) {
        client.ReadSize = false;
        break ;
      }
      else {
        client.chunkData = client.data.substr(0, client.chunkSize);
        if (!client.chunkData.empty())
          writeToFile(client.chunkData, client.file_fd);
        client.data = client.data.substr(client.chunkSize + 2);
        client.ReadSize = true;

        client.pos = client.data.find(client.boundary + "--");
        if (client.pos != std::string::npos && client.data.find(client.boundary + "\r\n") == std::string::npos) {
          std::cerr << "end boundary found" << std::endl;
          client.bodyTaken = true;
          client.data.clear();
          return ;
        }
      }
    }
  }
}

bool takeBodyType(client_info& client) {
  if (client.method.empty() || !client.headersTaken || client.bodyTypeTaken)
    return true;

  client.isChunked = false;
  client.bodyTaken = false;
  client.ReadSize = true;
  client.chunkData = "";

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
        // client.boundary = "--" + client.boundary;
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
