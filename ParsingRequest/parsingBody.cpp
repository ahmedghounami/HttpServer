#include "../server.hpp"

//finished
void OtherData(client_info &client, std::map<int, server_config> &server) {

  if (client.ReadFlag == true) {
    client.ReadFlag = false;
    client.filename = nameGenerator(client.ContentType, client.upload_path, client.isCgi);
    client.file_fd = open(client.filename.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (client.file_fd == -1) {
      error_response(client, server[client.index_server], 500);//server error
      return ;
    }
    client.post_cgi_filename = client.filename;
    std::map<std::string, std::string>::iterator it = client.headers.find("content-length");
    if (it != client.headers.end()) {
      std::istringstream iss(it->second);
      iss >> client.chunkSize;
      if (client.chunkSize > server[client.index_server].max_body_size) {
        close(client.file_fd);
        std::cerr << "size exceeded" << std::endl;
        error_response(client, server[client.index_server], 413);//payload too large
        return ;
      } else if (client.chunkSize == 0) {
        close(client.file_fd);
        client.bodyTaken = true;
        return ;
      }
    } else {
      std::cerr << "content-length not found" << std::endl;
      close(client.file_fd);
      error_response(client, server[client.index_server], 411); //length required
      return ;
    }
  }

  while (!client.data.empty()) {
    if (client.chunkSize > 0) {
      if (!client.data.empty())
        writeToFile(client.data, client.file_fd);
      client.chunkSize -= client.data.size();
      client.data.clear();
    }
    if (client.chunkSize == 0) {
      close(client.file_fd);
      client.bodyTaken = true;
      client.data.clear();
      return ;
    }
  }
}

//finished
void ChunkedOtherData(client_info& client, std::map<int, server_config> &server) {

  if (client.isCgi) {
    if (client.file_fd == -42) {
      client.filename = nameGenerator(client.ContentType, client.upload_path, client.isCgi);
      client.file_fd = open(client.filename.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0644);
      if (client.file_fd == -1) {
        error_response(client, server[client.index_server], 500);// server error
        return ;
      }
      client.post_cgi_filename = client.filename;
    }
    if (client.data.find("0\r\n\r\n") != std::string::npos) {
      client.bodyTaken = true;
    }
    if (!client.data.empty())
      writeToFile(client.data, client.file_fd);
    client.data.clear();
    if (client.bodyTaken == true)
      close(client.file_fd);
    return ;
  }

  while (!client.data.empty()) {

    if (client.ReadFlag ==  true) {
      if (client.file_fd == -42) {
        client.filename = nameGenerator(client.ContentType, client.upload_path, client.isCgi);
        client.file_fd = open(client.filename.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0644);
        if (client.file_fd == -1) {
          error_response(client, server[client.index_server], 500);//is it 500?
          return ;
        }
      client.post_cgi_filename = client.filename;
      }
      client.pos = client.data.find("\r\n");//check wether it is found or not
      std::string ChunkSizeString = client.data.substr(0, client.pos);
      client.data = client.data.substr(client.pos + 2);
      std::istringstream iss(ChunkSizeString);
      client.chunkSize = 0;
      iss >> std::hex >> client.chunkSize;
      client.FileSize += client.chunkSize;
      if (client.FileSize > server[client.index_server].max_body_size) {
        std::cerr << "size exceeded" << std::endl;
        close(client.file_fd);
        error_response(client, server[client.index_server], 413);//payload too large
        return ;
      } else if (client.chunkSize == 0) {
        close(client.file_fd);
        client.bodyTaken = true;
        return ;
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
          close(client.file_fd);
          client.bodyTaken = true;
          client.data.clear();
        }
      }
    }
  }
}

//finished
void ChunkedFormData(client_info& client, std::map<int, server_config> &server) {
  
  if (client.isCgi) {
    if (client.file_fd == -42) {
      client.filename = nameGenerator(client.ContentType, client.upload_path, client.isCgi);
      client.file_fd = open(client.filename.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0644);
      if (client.file_fd == -1) {
        error_response(client, server[client.index_server], 500);//is it 500?
        return ;
      }
      client.post_cgi_filename = client.filename;
    }
    if (client.data.find(client.boundary + "--") != std::string::npos)
      client.bodyTaken = true;
    if (!client.data.empty())
      writeToFile(client.data, client.file_fd);
    client.data.clear();
    if (client.bodyTaken == true)
      close(client.file_fd);
    return ;
  }

  while (!client.data.empty()) {

    client.pos = client.data.find(client.boundary + "\r\n");
    if (client.pos != std::string::npos && client.pos <= 10 && client.ReadFlag == true) {//attention here
      client.data = client.data.substr(client.pos);
      if (client.data.find("\r\n", client.boundary.size() + 2) == std::string::npos)
        break;
      if (NewFile(client, server)) {
        return ;
      }
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
        std::cerr << "size exceeded" << std::endl;
        close(client.file_fd);
        error_response(client, server[client.index_server], 413);//payload too large
        return ;
      }
      if (client.data.find(client.boundary + "\r\n") == std::string::npos
          && client.data.find(client.boundary + "--") != std::string::npos
          && client.data.size() <= 65) {
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
          close(client.file_fd);
          client.bodyTaken = true;
          client.data.clear();
          return ;
        }
      }
    }
  }
}

//finished
void FormData(client_info& client, std::map<int, server_config> &server) {

  if (client.isCgi) {
    if (client.file_fd == -42) {
      client.filename = nameGenerator(client.ContentType, client.upload_path, client.isCgi);
      client.file_fd = open(client.filename.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0644);
      if (client.file_fd == -1) {
        error_response(client, server[client.index_server], 500);//is it 500?
        return ;
      }
      client.post_cgi_filename = client.filename;
    }
    if (client.data.find(client.boundary + "--") != std::string::npos)
      client.bodyTaken = true;
    if (!client.data.empty())
      writeToFile(client.data, client.file_fd);
    client.data.clear();
    if (client.bodyTaken == true)
      close(client.file_fd);
    return ;
  }

  while (!client.data.empty()) {

    client.pos = client.data.find(client.boundary + "\r\n");
    if (client.pos != std::string::npos && client.ReadFlag == true) {
      if (client.data.find("\r\n", client.boundary.size() + 2) == std::string::npos) {
        break;
      }
      if (NewFile(client, server)) {
        return ;
      }
      client.ReadFlag = false;
      if (client.data.empty()) {
        break ;
      }
    }

    client.pos = client.data.find("\r\n" + client.boundary);
    if (client.pos != std::string::npos) {
      client.ReadFlag = true;
      client.chunkData = client.data.substr(0, client.pos);
      client.FileSize += client.chunkData.size();
      if (client.FileSize > server[client.index_server].max_body_size) {
        std::cerr << "size exceeded" << std::endl;
        close(client.file_fd);
        error_response(client, server[client.index_server], 413);//payload too large
        return ;
      }
      if (!client.chunkData.empty())
        writeToFile(client.chunkData, client.file_fd);
      client.data = client.data.substr(client.pos + 2);
      
      client.pos = client.data.find(client.boundary + "--");
      if (client.pos != std::string::npos && client.pos == 0) {
        close(client.file_fd);
        client.bodyTaken = true;
        client.data.clear();
        return ;
      }
    } else if (!client.data.empty()) {
      client.FileSize += client.data.size();
      if (client.FileSize > server[client.index_server].max_body_size) {
        std::cerr << "size exceeded" << std::endl;
        close(client.file_fd);
        error_response(client, server[client.index_server], 413);//payload too large
        return ;
      }
      writeToFile(client.data, client.file_fd);
      client.data.clear();
      return ;
    }

  }
}
