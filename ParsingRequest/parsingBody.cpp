/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   parsingBody.cpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hboudar <hboudar@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/20 00:16:24 by hboudar           #+#    #+#             */
/*   Updated: 2025/04/17 23:32:46 by hboudar          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../server.hpp"

void ChunkedOtherData(client_info &client) {

  while (!client.data.empty() && client.isChunked && !client.bodyTaken) {
    
    //here we look for the end of the chunked data
    client.pos = client.data.find("0\r\n\r\n");
    if (client.pos != std::string::npos && client.pos == 0) {
      client.bodyTaken = true;
      std::cerr << "end of chunked data" << std::endl;
      return ;
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
      std::cerr << "ChunkSize: " << client.chunkSize << std::endl;
    }

    //here we read data if we have it if not we break
    if (client.chunkSize > 0) {
      if (client.chunkSize + 2 > client.data.size()) {
        client.ReadSize = false;
        return ;
      }
      else {
        client.chunkData = client.data.substr(0, client.chunkSize);
        if (!client.chunkData.empty())
          writeToFile(client.chunkData, client.file_fd);
        // std::cerr << "|" << client.chunkData << "|" << std::endl;
        client.data = client.data.substr(client.chunkSize + 2);
        client.ReadSize = true;
      }
    }
  
  }
}

void ChunkedFormData(client_info& client) {
  
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
        client.bodyTypeTaken = 1;// formDataChunked(client);
      } else {
        client.bodyTypeTaken = 2;// otherDataChunked(client);
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
        client.bodyTypeTaken = 3;// formData(client);

      } else {
        client.bodyTypeTaken = 4;// otherData(client);
      }
    } else { // No content-type header
      std::cerr << "Error: Missing 'Content-Type' header" << std::endl;
      return false; //respond and clear client;
    }
  }
  return true;
}
