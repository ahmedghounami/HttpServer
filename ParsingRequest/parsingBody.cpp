#include "../server.hpp"

void OtherData(client_info &client, std::map<int, server_config> &server) {

  // if (handlepathinfo(client, server) == true) {
  //   if (client.ReadSize == true) {
  //     std::map<std::string, std::string>::iterator it = client.headers.find("content-length");
  //     if (it != client.headers.end()) {
  //       std::istringstream iss(it->second);
  //       iss >> client.chunkSize;
  //     } else {
  //       std::cerr << "content-length not found" << std::endl;
  //       error_response(client, server[client.index_server], 411);//is it 411?
  //       return ;
  //     }
  //     client.ReadSize = false;
  //   }
  //   if (client.chunkSize == 0)
  //     client.bodyTaken = true;//ga3ma triturni 7ta yqra
  //   // hna qra;
  // }

  while (!client.data.empty()) {
    if (client.ReadFlag == true) {
      client.ReadFlag = false;
      client.file_fd = open("www/RB", O_WRONLY | O_CREAT | O_TRUNC, 0644);
      if (client.file_fd == -1) {
        std::cerr << "Error opening file" << std::endl;
        return ;
      }
      std::map<std::string, std::string>::iterator it = client.headers.find("content-length");
      if (it != client.headers.end()) {
        std::istringstream iss(it->second);
        iss >> client.chunkSize;
        std::cerr << "client.chunkSize: " << client.chunkSize << std::endl;
      } else {
        std::cerr << "content-length not found" << std::endl;
        error_response(client, server[client.index_server], 411);//is it 411?
        return ;
      }
    }
    
    //keep reading from client.data until we reach the content-length
    if (client.chunkSize > 0) {
      if (!client.data.empty())
        writeToFile(client.data, client.file_fd);
      client.chunkSize -= client.data.size();
      client.data.clear();
    }
    if (client.chunkSize == 0) {
      std::cerr << "data cleared." << std::endl;
      client.bodyTaken = true;
      client.data.clear();
      return ;
    }
    
  }
}

void FormData(client_info& client, std::map<int, server_config> &server) {

  // if (handlepathinfo(client, server) == true) {
  //   if (client.data.find("0\r\n\r\n") != std::string::npos)
  //     client.bodyTaken = true;
  //   //3amer lfile
  // }
  while (!client.data.empty()) {

    client.pos = client.data.find(client.boundary + "\r\n");
    if (client.pos != std::string::npos && client.ReadFlag == true) {
      if (client.data.find("\r\n", client.boundary.size() + 2) == std::string::npos) {
        std::cerr << "'CRLF' found." << std::endl;
        break;
      }
      NewFile(client, server);
      client.ReadFlag = false;
      if (client.data.empty()) {
        std::cerr << "data is empty." << std::endl;
        break ;
      }
    }

    client.pos = client.data.find("\r\n" + client.boundary);
    if (client.pos != std::string::npos) {
      client.ReadFlag = true;
      client.chunkData = client.data.substr(0, client.pos);
      if (!client.chunkData.empty())
        writeToFile(client.chunkData, client.file_fd);
      client.data = client.data.substr(client.pos + 2);

      
      client.pos = client.data.find(client.boundary + "--");
      if (client.pos != std::string::npos && client.pos == 0) {
        std::cerr << "end boundary found |" << client.data << "|" << std::endl;
        close(client.file_fd);
        client.file_fd = -42;
        client.bodyTaken = true;
        client.data.clear();
        return ;
      }
    } else if (!client.data.empty()) {
      writeToFile(client.data, client.file_fd);
      client.data.clear();
      return ;
    }

  }

}

void ChunkedOtherData(client_info& client, std::map<int, server_config> &server) {

  // if (handlepathinfo(client, server) == true) {
  //   if (client.data.find("0\r\n") != std::string::npos)
  //     client.bodyTaken = true;
  //   //3amer lfile
  // }

  while (!client.data.empty()) {

    if (client.ReadFlag ==  true) {//move this part to takeBodyType function
      client.ReadFlag = false;
      client.pos = client.data.find("\r\n");
      std::string ChunkSizeString = client.data.substr(0, client.pos);
      client.data = client.data.substr(client.pos + 2);
      std::istringstream iss(ChunkSizeString);
      client.chunkSize = 0;
      iss >> std::hex >> client.chunkSize;
      std::string fileName = "www/Chunked_RB." + client.ContentType.substr(client.ContentType.find("/") + 1);
      std::cerr << "fileName: " << fileName << std::endl;
      client.file_fd = open(fileName.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0644);
      if (client.file_fd == -1) {
        std::cerr << "Error opening file" << std::endl;
        return ;//
        error_response(client, server[client.index_server], 500);//is it 500?
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

void ChunkedFormData(client_info& client, std::map<int, server_config> &server) {
  
  // if (handlepathinfo(client)) {
  //   if (client.data.find("0\r\n\r\n") != std::string::npos)
  //     client.bodyTaken = true;
      
  // }

  while (!client.data.empty()) {

    client.pos = client.data.find(client.boundary + "\r\n");
    if (client.pos != std::string::npos && client.pos <= 10 && client.ReadFlag == true) {//attention here
      client.data = client.data.substr(client.pos);
      if (client.data.find("\r\n", client.boundary.size() + 2) == std::string::npos)
        break;
      NewFile(client, server);
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
      // std::cerr << "client.chunkSize: " << client.chunkSize << std::endl;
      if (client.data.find(client.boundary + "\r\n") == std::string::npos
          && client.data.find(client.boundary + "--") != std::string::npos
          && client.data.size() <= 65) {
        std::cerr << "data ended |" << client.data << "|" << std::endl;
        client.bodyTaken = true;
        close(client.file_fd);
        return ;
      }
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

        if (client.data.find(client.boundary + "--") != std::string::npos && client.data.size() <= 69) {
          std::cerr << "clean ending |" << client.data << "|" << std::endl;
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