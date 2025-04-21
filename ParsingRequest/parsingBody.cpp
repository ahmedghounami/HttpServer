#include "../server.hpp"

void FormData(client_info& client) {

  while (!client.data.empty()) {

    client.pos = client.data.find(client.boundary + "\r\n");
    if (client.pos != std::string::npos && client.ReadFlag == true) {
      client.data = client.data.substr(client.pos);
      if (client.data.find("\r\n", client.boundary.size() + 2) == std::string::npos) {
        std::cerr << "breaking from loop : 'CRLF' found." << std::endl;
        break;
      }
      NewFile(client);
      client.ReadFlag = false;
      if (client.data.empty()) {
        std::cerr << "breaking from loop : data is empty." << std::endl;
        break ;
      }
    }

    //client.data filled with : only data or data with another boundary
    client.pos = client.data.find("\r\n");
    if (client.pos != std::string::npos) {
      client.chunkData = client.data.substr(0, client.pos);
      client.data = client.data.substr(client.pos + 2);
      if (!client.chunkData.empty())
        writeToFile(client.chunkData, client.file_fd);
      client.chunkData.clear();
      client.ReadFlag = true;

      client.pos = client.data.find(client.boundary + "--");
      if (client.pos != std::string::npos && client.pos == 0) {
        std::cerr << "end boundary found " << std::endl;
        close(client.file_fd);
        client.file_fd = -42;
        client.bodyTaken = true;
        client.data.clear();
        std::cerr << "data cleared." << std::endl;  
        return ;
      }
    } else if (!client.data.empty()) {
        writeToFile(client.data, client.file_fd);
      client.data.clear();
    }
  }
  std::cerr << "the data is empty." << std::endl;
}

void ChunkedOtherData(client_info& client) {

  while (!client.data.empty()) {

    if (client.ReadFlag ==  true) {//move this part to takeBodyType function
      client.ReadFlag = false;
      client.pos = client.data.find("\r\n");
      std::string ChunkSizeString = client.data.substr(0, client.pos);
      client.data = client.data.substr(client.pos + 2);
      std::istringstream iss(ChunkSizeString);
      client.chunkSize = 0;
      iss >> std::hex >> client.chunkSize;
      if (client.file_fd == -42) {
        std::string fileName = "raw-binary_File." + client.ContentType.substr(client.ContentType.find("/") + 1);
        std::cerr << "fileName: " << fileName << std::endl;
        client.file_fd = open(fileName.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0644);
        if (client.file_fd == -1) {
          std::cerr << "Error opening file" << std::endl;
          return ;
        }
      }
    }

    if (client.chunkSize > 0) {
      if (client.chunkSize + 2 > client.data.size()) {
        if (!client.data.empty())
          writeToFile(client.data, client.file_fd);
        client.chunkSize -= client.data.size();
        client.data.clear();
      } else {
        client.chunkData = client.data.substr(0, client.chunkSize);
        if (!client.chunkData.empty())
          writeToFile(client.chunkData, client.file_fd);
        client.data = client.data.substr(client.chunkSize + 2);
        client.chunkSize = 0;
        client.ReadFlag = true;

        client.pos = client.data.find("0\r\n");
        if (client.pos != std::string::npos) {
          std::cerr << "end of Raw data |" << client.data << "|" << std::endl;
          close(client.file_fd);
          client.file_fd = -42;
          client.bodyTaken = true;
          client.data.clear();
        std::cerr << "data cleared." << std::endl;  
        }
      }
    }
  }
}

void ChunkedFormData(client_info& client) {

  while (!client.data.empty()) {

    client.pos = client.data.find(client.boundary + "\r\n");
    if (client.pos != std::string::npos && client.ReadFlag == true) {
      client.data = client.data.substr(client.pos);
      if (client.data.find("\r\n", client.boundary.size() + 2) == std::string::npos) {
        std::cerr << "breaking from loop." << std::endl;
        break;
      }
      NewFileChunked(client);
      if (client.data.empty())
        break ;
    }

    if (client.ReadFlag ==  true) {
      client.ReadFlag = false;
      client.pos = client.data.find("\r\n");
      std::string ChunkSizeString = client.data.substr(0, client.pos);
      client.data = client.data.substr(client.pos + 2);
      std::istringstream iss(ChunkSizeString);
      client.chunkSize = 0;
      iss >> std::hex >> client.chunkSize;
    }

    if (client.chunkSize > 0) {
      if (client.chunkSize + 2 > client.data.size()) {
        client.ReadFlag = false;
        break ;
      }
      else {
        client.chunkData = client.data.substr(0, client.chunkSize);
        if (!client.chunkData.empty())
          writeToFile(client.chunkData, client.file_fd);
        client.data = client.data.substr(client.chunkSize + 2);
        client.ReadFlag = true;
        client.chunkSize = 0;

        client.pos = client.data.find(client.boundary + "--");
        if (client.pos != std::string::npos && client.data.find(client.boundary + "\r\n") == std::string::npos) {
          std::cerr << "end boundary found |" << client.data << "|" << std::endl;
          close(client.file_fd);
          client.file_fd = -42;
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

  std::map<std::string, std::string>::iterator it = client.headers.find("transfer-encoding");
  if (it != client.headers.end() && it->second == "chunked") {
    it = client.headers.find("content-type");
    if (it != client.headers.end()) {
      client.ContentType = it->second;
      if (client.ContentType.find("multipart/form-data") != std::string::npos) {
        client.boundary = getBoundary(client.ContentType);
        if (client.boundary.empty()) {
          client.boundary.clear();
          bad_request(client);
          return false;
        }
        client.bodyTypeTaken = 1;// formDataChunked(client);
      } else {
        client.bodyTypeTaken = 2;// otherDataChunked(client);
      }
    } else {
      bad_request(client);
      return false;
    }
  } else {
    it = client.headers.find("content-type");
    if (it != client.headers.end()) {
      client.ContentType = it->second;
      if (client.ContentType.find("multipart/form-data") != std::string::npos) {
        client.boundary = getBoundary(client.ContentType);
        if (client.boundary.empty()) {
          client.boundary.clear();
          bad_request(client);
          return false;
        }
        client.bodyTypeTaken = 3;// formData(client);

      } else {
        client.bodyTypeTaken = 4;// otherData(client);
      }
    } else {
      bad_request(client);
      return false;
    }
  }
  return true;
}
