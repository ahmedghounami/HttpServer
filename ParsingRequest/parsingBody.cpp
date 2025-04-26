#include "../server.hpp"

void OtherData(client_info &client, std::map<int, server_config> &server) {

  while (!client.data.empty()) {
    if (client.ReadFlag == true) {
      client.ReadFlag = false;
      std::string filename = nameGenerator(client.ContentType, client.upload_path);
      client.file_fd = open(filename.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0644);
      if (client.file_fd == -1) {
        error_response(client, server[client.index_server], 500);//server error
        return ;
      }
      std::map<std::string, std::string>::iterator it = client.headers.find("content-length");
      if (it != client.headers.end()) {
        std::istringstream iss(it->second);
        iss >> client.chunkSize;
        if (client.chunkSize > server[client.index_server].max_body_size) {
          std::cerr << "client.chunkSize: " << client.chunkSize << std::endl;
          std::cerr << "confing size: " << server[client.index_server].max_body_size << std::endl;
          error_response(client, server[client.index_server], 413);//payload too large
          return ;
        }
      } else {
        std::cerr << "content-length not found" << std::endl;
        error_response(client, server[client.index_server], 411); //length required
        return ;
      }
    }
    
    if (client.chunkSize > 0) {
      if (!client.data.empty())
        writeToFile(client.data, client.file_fd);
      client.chunkSize -= client.data.size();
      std::cerr << "after reading chunkSize: " << client.chunkSize << std::endl;
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

void ChunkedOtherData(client_info& client, std::map<int, server_config> &server) {

  if (client.isCgi) {
    if (client.file_fd == -42) {
      std::string fileName = nameGenerator(client.ContentType, client.upload_path);
      client.file_fd = open(fileName.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0644);
      if (client.file_fd == -1) {
        error_response(client, server[client.index_server], 500);// server error
        return ;
      }
    }
    if (client.data.find("0\r\n\r\n") != std::string::npos) {
      std::cerr << "ending for cgi was found" << std::endl;
      client.bodyTaken = true;
    }
    if (!client.data.empty())
      writeToFile(client.data, client.file_fd);
    client.data.clear();
    return ;
  }

  while (!client.data.empty()) {

    if (client.ReadFlag ==  true) {
      client.pos = client.data.find("\r\n");
      std::string ChunkSizeString = client.data.substr(0, client.pos);
      client.data = client.data.substr(client.pos + 2);
      std::istringstream iss(ChunkSizeString);
      client.chunkSize = 0;
      iss >> std::hex >> client.chunkSize;
      client.FileSize += client.chunkSize;
      if (client.FileSize > server[client.index_server].max_body_size) {
        std::cerr << "confing size: " << server[client.index_server].max_body_size << std::endl;
        std::cerr << "client.FileSize: " << client.FileSize << std::endl;
        error_response(client, server[client.index_server], 413);//payload too large
        return ;
      }
      if (client.file_fd == -42) {
        std::string fileName = nameGenerator(client.ContentType, client.upload_path);
        client.file_fd = open(fileName.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0644);
        if (client.file_fd == -1) {
          error_response(client, server[client.index_server], 500);//is it 500?
          return ;
        }
      }
      client.ReadFlag = false;
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

        if (client.data.find("0\r\n\r\n") != std::string::npos && client.data.size() <= 5) {
          std::cerr << "the ending was found of Raw data |" << client.data << "|" << std::endl;
          close(client.file_fd), client.file_fd = -42;
          client.bodyTaken = true;
          client.data.clear();
        }
      }
    }
  }
}

void ChunkedFormData(client_info& client, std::map<int, server_config> &server) {
  
  if (client.isCgi) {
    if (client.file_fd == -42) {
      std::string fileName = nameGenerator(client.ContentType, client.upload_path);
      std::cerr << "fileName: " << fileName << std::endl;
      client.file_fd = open(fileName.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0644);
      if (client.file_fd == -1) {
        error_response(client, server[client.index_server], 500);//is it 500?
        return ;
      }
    }
    if (client.data.find(client.boundary + "--") != std::string::npos)
      client.bodyTaken = true;
    if (!client.data.empty())
      writeToFile(client.data, client.file_fd);
    client.data.clear();
    return ;
  }

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
      client.FileSize += client.chunkSize;
      if (client.FileSize > server[client.index_server].max_body_size) {
        std::cerr << "confing size: " << server[client.index_server].max_body_size << std::endl;
        std::cerr << "client.FileSize: " << client.FileSize << std::endl;
        error_response(client, server[client.index_server], 413);//payload too large
        return ;
      }
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

void FormData(client_info& client, std::map<int, server_config> &server) {

  if (client.isCgi) {
    if (client.file_fd == -42) {
      std::string filename = nameGenerator(client.ContentType, client.upload_path);
      client.file_fd = open(filename.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0644);
      if (client.file_fd == -1) {
        std::cerr << "Error opening file" << std::endl;
        error_response(client, server[client.index_server], 500);//is it 500?
        return ;
      }
    }
    if (client.data.find(client.boundary + "--") != std::string::npos)
      client.bodyTaken = true;
    if (!client.data.empty())
      writeToFile(client.data, client.file_fd);
    client.data.clear();
    return ;
  }

  while (!client.data.empty()) {

    client.pos = client.data.find(client.boundary + "\r\n");
    if (client.pos != std::string::npos && client.ReadFlag == true) {
      if (client.data.find("\r\n", client.boundary.size() + 2) == std::string::npos) {
        // std::cerr << "'CRLF' found." << std::endl;
        break;
      }
      NewFile(client, server);
      client.ReadFlag = false;
      if (client.data.empty()) {
        // std::cerr << "data is empty." << std::endl;
        break ;
      }
    }

    client.pos = client.data.find("\r\n" + client.boundary);
    if (client.pos != std::string::npos) {
      client.ReadFlag = true;
      client.chunkData = client.data.substr(0, client.pos);
      client.FileSize += client.chunkData.size();
      if (client.FileSize > server[client.index_server].max_body_size) {
        std::cerr << "confing size: " << server[client.index_server].max_body_size << std::endl;
        std::cerr << "client.FileSize: " << client.FileSize << std::endl;
        error_response(client, server[client.index_server], 413);//payload too large
        return ;
      }
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
      client.FileSize += client.data.size();
      if (client.FileSize > server[client.index_server].max_body_size) {
        std::cerr << "confing size: " << server[client.index_server].max_body_size << std::endl;
        std::cerr << "client.FileSize: " << client.FileSize << std::endl;
        error_response(client, server[client.index_server], 413);//payload too large
        return ;
      }
      writeToFile(client.data, client.file_fd);
      client.data.clear();
      return ;
    }

  }
}
