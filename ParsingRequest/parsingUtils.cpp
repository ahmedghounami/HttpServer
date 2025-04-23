#include "../server.hpp"

std::string trim(const std::string &str) {
  size_t first = str.find_first_not_of(" \t");
  if (first == std::string::npos)
    return "";
  size_t last = str.find_last_not_of(" \t");
  return str.substr(first, last - first + 1);
}

bool isValidHeaderKey(const std::string &key) {
  if (key.empty())
    return false;
  for (size_t i = 0; i < key.length(); ++i) {
    if (!isalpha(key[i]) && key[i] != '-' && key[i] != '_')
      return false;
  }
  return true;
}

bool isValidHeaderValue(const std::string &value) {
  for (size_t i = 0; i < value.length(); ++i) {
    if (iscntrl(value[i]) && value[i] != '\t') {
      return false;
    }
  }
  return true;
}

std::string toLower(const std::string& str) {
    std::string lowerStr = str;
    for (size_t i = 0; i < lowerStr.length(); ++i) {
        lowerStr[i] = std::tolower(lowerStr[i]);
    }
    return lowerStr;
}

std::string getBoundary(const std::string &contentType) {
  size_t pos = contentType.find("boundary=");
  if (pos != std::string::npos)
    return contentType.substr(pos + 9);
  return "";
}

void writeToFile(std::string &body, int fd) {
  if (fd > 0) {
    if (body.empty() || (body.size() == 2 && body == "\r\n"))
      return ;
    write(fd, body.c_str(), body.size());
  }
}

std::string nameGenerator() {
  std::string name;
  const char charset[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
  name += "default_";
  for (int i = 0; i < 5; ++i) {
    int index = rand() % (sizeof(charset) - 1);
    name += charset[index];
  }
  name += ".txt";
  return name;
}

static void ParseContentDisposition(client_info& client, std::map<int, server_config>& server) {
  std::cerr << "-------------ParseContentDisposition------------" << std::endl;
  client.pos = client.data.find("name=\"");
  client.data = client.data.substr(client.pos + 6, client.data.size());//problem
  std::string sub = client.data.substr(0, client.data.find("\""));
  client.data = client.data.substr(client.data.find("\"") + 3 , client.data.size());
  client.name = sub;
  close(client.file_fd);
  client.file_fd = -42;

  client.pos = client.data.find("filename=\"");
  if (client.pos == std::string::npos || client.pos != 0)
  {
    // close(client.file_fd);
    client.filename.clear();
    client.filename = nameGenerator();
    client.file_fd = open(client.filename.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (client.file_fd == -1) {
      std::cerr << "Error opening file" << std::endl;
      return ;//
      error_response(client, server[client.index_server], 500);//is it 500?
    }
    client.data = client.data.substr(2 + (client.pos != 0 && client.bodyTypeTaken != 3) * 2, client.data.size());
  } else if (client.pos != std::string::npos) {
    // close(client.file_fd);
    client.data = client.data.substr(client.pos + 10, client.data.size());
    sub = client.data.substr(0, client.data.find("\""));
    client.data = client.data.substr(client.data.find("\"") + 3 , client.data.size());
    client.filename = sub;
    client.file_fd = open(client.filename.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (client.file_fd == -1) {
      std::cerr << "Error opening file" << std::endl;
      return ;//
      error_response(client, server[client.index_server], 500);//is it 500?
    }
  }
  std::cerr << "name: |" << client.name << "|" << std::endl;
  std::cerr << "filename: |" << client.filename << "|" << std::endl;
  std::cerr << "-------------End ParseContentDisposition--------" << std::endl;
}

static void ParseContentType(client_info& client) {
  std::cerr << "--------------ParseContentType----------------" << std::endl;
  client.data = client.data.substr(client.pos + 14, client.data.size());
  client.pos = client.data.find("/");
  client.data = client.data.substr(client.pos + 1, client.data.size());
  client.contentTypeform = client.data.substr(0, client.data.find("\r\n"));
  if (client.bodyTypeTaken == 1 || client.bodyTypeTaken == 2)
    client.data = client.data.substr(client.data.find("\r\n") + 6, client.data.size());
  else
    client.data = client.data.substr(client.data.find("\r\n") + 4, client.data.size());

  std::cerr << "Content-Type: |" << client.contentTypeform << "|" << std::endl;
  std::cerr << "--------------End ParseContentType-------------" << std::endl;
}

void NewFile(client_info &client, std::map<int, server_config> &server) {
  client.chunkData = "", client.chunkSize = 0;// close(client.file_fd);
  client.data = client.data.substr(client.boundary.size() + 2);

  client.pos = client.data.find("Content-Disposition: form-data;");
  if (client.pos != std::string::npos && client.pos == 0) {
    client.data = client.data.substr(client.pos + 32, client.data.size());
    ParseContentDisposition(client, server);
  }

  client.pos = client.data.find("Content-Type:");
  if (client.pos != std::string::npos && client.pos == 0) {
    ParseContentType(client);
  }
}