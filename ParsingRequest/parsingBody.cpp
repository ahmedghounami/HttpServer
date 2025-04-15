/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   parsingBody.cpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hboudar <hboudar@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/20 00:16:24 by hboudar           #+#    #+#             */
/*   Updated: 2025/04/14 19:51:38 by hboudar          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../server.hpp"

 
void ParseContentDisposition(client_info& client) {
  client.data = client.data.substr(client.pos + 32, client.data.size());
  client.pos = client.data.find("name=\"");
  client.data = client.data.substr(client.pos + 6, client.data.size());
  std::string sub = client.data.substr(0, client.data.find("\""));
  client.data = client.data.substr(client.data.find("\"") + 3 , client.data.size());
  client.name = sub;

  client.pos = client.data.find("filename=\"");
  if (client.pos == std::string::npos)
  {
    // no filename
    client.filename = "";
    client.data = client.data.substr(2, client.data.size());
    
  } else if (client.pos != std::string::npos) {
    // filename
    client.data = client.data.substr(client.pos + 11, client.data.size());
    sub = client.data.substr(0, client.data.find("\""));
    client.data = client.data.substr(client.data.find("\"") + 3 , client.data.size());
    client.filename = sub;
    if (client.filename.empty())
      client.filename = "default_file";
  }
  std::cerr << "name: |" << client.name << "|" << std::endl;
  std::cerr << "filename: |" << client.filename << "|" << std::endl;
}

void ParseContentType(client_info& client) {
  client.data = client.data.substr(client.pos + 14, client.data.size());
  client.pos = client.data.find("/");
  client.data = client.data.substr(client.pos + 1, client.data.size());
  client.contentTypeform = client.data.substr(0, client.data.find("\n" - 1));
  client.data = client.data.substr(client.data.find("\n") + 3, client.data.size());
  if (client.filename.empty()) {
    /*
      if (this->MimeTypeMap.find(this->MimeType) != this->MimeTypeMap.end())
          this->FileName = generate_random_string(5) + this->MimeTypeMap[this->MimeType];
      else
          this->FileName = generate_random_string(5) + ".bin";
    */
  }
  std::cerr << "Content-Type: |" << client.contentTypeform << "|" << std::endl;
}

void ChunkedData(client_info& client) {
  if (client.data.find(client.boundary + "--") != std::string::npos) {
    std::cerr << "End of multipart data" << std::endl;
    client.bodyTaken = true;
    return ;  
  }
  if (client.bodyTaken || client.data.empty())
    return ;
  client.pos = client.data.find(client.boundary);
  if (client.pos == 0) {
    // close(client.file_fd);
    //client.openFile = false;
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
    std::cerr << "Chunk size: " << client.chunkSize << std::endl;
  
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

  std::cerr << "Parsing body" << std::endl;
  client.isChunked = false;
  client.bodyTaken = false;
  client.bytesLeft = 0;
  client.chunkData = "";

  client.file_fd = open("file", O_WRONLY | O_CREAT | O_TRUNC, 0644);
  if (client.file_fd == -1) {
    std::cerr << "Error: Failed to open file" << std::endl;
    exit (1);
  }

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

/*
    if (client.bodyTypeTaken == 1) {
      pos = client.data.find("\r\n\r\n");
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
*/