/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   parsingBody.cpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hboudar <hboudar@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/20 00:16:24 by hboudar           #+#    #+#             */
/*   Updated: 2025/04/17 22:37:22 by hboudar          ###   ########.fr       */
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
      std::cerr << "ChunkSize: " << client.chunkSize  << std::endl;
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
        std::cerr << "ChunkData: |" << client.chunkData  << "|" << std::endl;
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


/*//   size_t pos = client.data.find(client.boundary + "--");//end boundary

  //   if (pos != std::string::npos && client.pos == std::string::npos && client.bytesLeft == 0) {
  //     std::cerr << "end boundary found" << std::endl;
  //     std::cerr << "Data: |" << client.data << "|" << std::endl;
  //     client.bodyTaken = true;
  //     break ;
  //   }
  
  //   else {
  //     if (client.bytesLeft > 0 && client.data.size() >= client.bytesLeft) {

  //       client.chunkData = client.data.substr(0, client.bytesLeft);
  //       if (!client.chunkData.empty())
  //         writeToFile(client.chunkData, client.file_fd);
  //       client.data = client.data.substr(client.bytesLeft);
  //       client.bytesLeft = 0;

  //     } else if (client.bytesLeft > 0) {

  //       client.chunkData = client.data.substr(0, client.data.size());
  //       if (!client.chunkData.empty())
  //         writeToFile(client.chunkData, client.file_fd);
  //       client.data.clear();
  //       client.bytesLeft -= client.chunkData.size();

  //     } else if (client.bytesLeft == 0) {

  //       client.pos = client.data.find("\r\n");
  //       std::string ChunkSizeString = client.data.substr(0, client.pos);
  //       client.data = client.data.substr(client.pos + 2);
  //       std::istringstream iss(ChunkSizeString);
  //       client.chunkSize = 0;
  //       iss >> std::hex >> client.chunkSize;
  //       if (client.chunkSize + 2 > client.data.size()) {
  //         client.bytesLeft = client.chunkSize - (client.data.size() - 2);
  //         client.chunkData = client.data.substr(0, client.data.size() - 2);
  //         client.data.clear();
  //       } else {
  //         client.chunkData = client.data.substr(0, client.chunkSize);
  //         client.data = client.data.substr(client.chunkSize + 2);
  //         client.bytesLeft = 0;
  //       }
  //       if (!client.chunkData.empty())
  //         writeToFile(client.chunkData, client.file_fd);
  //       client.pos = client.data.find(client.boundary);
  //       if (client.pos != std::string::npos && client.bytesLeft == 0) {
  //         std::cerr << "before :|" << client.data << "|" << std::endl;
  //         client.data = client.data.substr(client.pos);
  //         std::cerr << "after :|" << client.data << "|" << std::endl;
  //       }
  //     }
  //   }

  //   if (client.data.empty())
  //     break ;*/