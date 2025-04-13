/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   parsingBody.cpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hboudar <hboudar@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/20 00:16:24 by hboudar           #+#    #+#             */
/*   Updated: 2025/04/13 20:30:28 by hboudar          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../server.hpp"

void formDataChunked(client_info& client) {
  FormPart part;
  size_t pos, chunkSize;
  std::string ChunkSizeString, chunkData;

  while (client.isChunked && !client.bodyTaken) {
    if (client.bytesLeft > 0 && client.data.size() >= client.bytesLeft) {
      chunkData = client.data.substr(0, client.bytesLeft);
      if (!chunkData.empty())
        write(client.file_fd, chunkData.c_str(), chunkData.size());
      client.data = client.data.substr(client.bytesLeft);
      client.bytesLeft = 0;
    } else if (client.bytesLeft > 0) {
      chunkData = client.data;
      if (!chunkData.empty())
        write(client.file_fd, chunkData.c_str(), chunkData.size());
      client.bytesLeft -= client.data.size();
      client.data.clear();
    }
    if (client.bodyTypeTaken == 1) {
      size_t pos = client.data.find("\r\n\r\n");
      if (pos != std::string::npos) {
        std::istringstream iss(client.data.substr(0, pos));
        std::string line;
        while (std::getline(iss, line)) {
          size_t start = line.find("filename=\"");
          if (start != std::string::npos) {
            start += 10;
            size_t end = line.find("\"", start);
            part.filename = line.substr(start, end - start);
          } else if (line.find("Content-Type:") != std::string::npos)
            part.contentType = line.substr(strlen("Content-Type: "));
        }
        client.formParts.push_back(part);
        client.data = client.data.substr(pos + 4);// for \r\n\r\n
      }
    }

    pos = client.data.find("0\r\n\r\n");
    if (pos != std::string::npos) {
     if (pos != 0) {
      chunkData = client.data.substr(0, pos);
      if (!chunkData.empty())
        write(client.file_fd, chunkData.c_str(), chunkData.size());
     } 
      client.bodyTaken = true;
      return ;
    }
    pos = client.data.find("\r\n");// for chunk size
    if (pos == 0) {
      client.data = client.data.substr(2);
      continue;
    }
    if (pos == std::string::npos)
      break;
    ChunkSizeString = client.data.substr(0, pos);
    std::istringstream iss(ChunkSizeString);
    chunkSize = 0;
    iss >> std::hex >> chunkSize;
    if (pos + 2 + chunkSize > client.data.length())
    {
        client.bytesLeft = chunkSize - (client.data.length() - pos - 2);
        chunkData = client.data.substr(pos + 2);
        client.data.clear();
    }
    else
    {
      chunkData = client.data.substr(pos + 2, chunkSize);
      client.data = client.data.substr(pos + 2 + chunkSize);
      client.bytesLeft = 0;
    }

    if (!chunkData.empty())
      write(client.file_fd, chunkData.c_str(), chunkData.size());

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
  chunkData = "";
  client.currentPos = 0;


  client.file_fd = open("file", O_WRONLY | O_CREAT | O_TRUNC, 0644);
  if (client.file_fd == -1) {
    std::cerr << "Error: Failed to open file" << std::endl;
    exit (1);
  }
  write(client.file_fd, "start here\n", 11);

  
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
        formDataChunked(client);
        client.bodyTypeTaken = 1;
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

