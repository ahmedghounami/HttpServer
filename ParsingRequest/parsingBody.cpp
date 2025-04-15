/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   parsingBody.cpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hboudar <hboudar@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/20 00:16:24 by hboudar           #+#    #+#             */
/*   Updated: 2025/04/15 15:25:45 by hboudar          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../server.hpp"

void ChunkedData(client_info& client) {
  if (client.data.find(client.boundary + "--") != std::string::npos) {
    // edge case : if the end boundary is not at the beginning of the data : which means there data bofre the end boundary that we need to write
    
    std::ofstream file("xyata");
    file << client.data;
    file.close();
    exit(1);
    std::cerr << "The body is complete" << std::endl;
    client.bodyTaken = true;
    return ;  
  }
  if (client.bodyTaken || client.data.empty())
    return ;
  client.pos = client.data.find(client.boundary);
  if (client.pos == 0) {
    //client.openFile = false;
    close(client.file_fd);
    client.chunkData = "";
    client.bytesLeft = 0;
    client.chunkSize = 0;
    client.pos = client.data.find("Content-Disposition: form-data;");
    if (client.pos != std::string::npos)
      ParseContentDisposition(client);
    client.pos = client.data.find("Content-Type:");
    if (client.pos != std::string::npos) {
      //open file = true
      ParseContentType(client);
    }
    
  }
    
  while (client.isChunked && !client.bodyTaken)
  {
    if (client.data.size() == 2 && client.data == "\r\n")
      break;
      
    if (client.bytesLeft > 0 && client.data.size() >= client.bytesLeft)
    {
      client.chunkData = client.data.substr(0, client.bytesLeft);

      if (!client.chunkData.empty())
        writeToFile(client.chunkData, client.file_fd);

      client.data = client.data.substr(client.bytesLeft);
      client.bytesLeft = 0;
    }
    else if (client.bytesLeft > 0)
    {
      client.chunkData = client.data;

      if (!client.chunkData.empty())
        writeToFile(client.chunkData, client.file_fd);

      client.bytesLeft -= client.chunkData.size();
      client.data.clear();
    }

    client.pos = client.data.find("0\r\n\r\n");
    if (client.pos != std::string::npos)
    {
      if (client.pos != 0)
      {
        client.chunkData = client.data.substr(0, client.pos);
        
        if (!client.chunkData.empty())
          writeToFile(client.chunkData, client.file_fd);
      } 
      client.bodyTaken = true;
      return ;
    }

    client.pos = client.data.find("\r\n");
    if (client.pos == 0)
    {
      client.data = client.data.substr(2);
      continue;
    }
    if (client.pos == std::string::npos)
      break;

    std::string ChunkSizeString = client.data.substr(0, client.pos);
    std::istringstream iss(ChunkSizeString);
    client.chunkSize = 0;
    iss >> std::hex >> client.chunkSize;
  
    if (client.pos + 2 + client.chunkSize > client.data.length())
    {
        client.bytesLeft = client.chunkSize - (client.data.length() - client.pos - 2);
        client.chunkData = client.data.substr(client.pos + 2);
        client.data.clear();
    }
    else
    {
      client.chunkData = client.data.substr(client.pos + 2, client.chunkSize);
      client.data = client.data.substr(client.pos + 2 + client.chunkSize);
      client.bytesLeft = 0;
    }

    if (!client.chunkData.empty())
      writeToFile(client.chunkData, client.file_fd);

    if (client.data.empty())
      break;
  }
}

bool takeBody(client_info& client) {
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
        size_t pos = client.data.find("\r\n");
        if (pos != std::string::npos)
          client.data = client.data.substr(pos + 2);
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
