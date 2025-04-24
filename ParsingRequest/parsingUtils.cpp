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

std::string nameGenerator(std::string MimeType) {

  std::cerr << "-------------nameGenerator----------------" << std::endl;
  std::map<std::string, std::string> MimeTypeMap;

  MimeTypeMap["application/octet-stream"] = ".bin";
  MimeTypeMap["application/json"] = ".json";
  MimeTypeMap["application/xml"] = ".xml";
  MimeTypeMap["application/zip"] = ".zip";
  MimeTypeMap["application/gzip"] = ".gz";
  MimeTypeMap["application/x-tar"] = ".tar";
  MimeTypeMap["application/x-7z-compressed"] = ".7z";
  MimeTypeMap["application/pdf"] = ".pdf";
  MimeTypeMap["application/x-www-form-urlencoded"] = ".txt";
  MimeTypeMap["application/x-bzip"] = ".bz";
  MimeTypeMap["application/x-bzip2"] = ".bz2";
  MimeTypeMap["application/x-rar-compressed"] = ".rar";
  MimeTypeMap["application/x-msdownload"] = ".exe";
  MimeTypeMap["application/vnd.ms-excel"] = ".xls";
  MimeTypeMap["application/vnd.openxmlformats-officedocument.spreadsheetml.sheet"] = ".xlsx";
  MimeTypeMap["text/plain"] = ".txt";
  MimeTypeMap["text/html"] = ".html";
  MimeTypeMap["text/css"] = ".css";
  MimeTypeMap["text/csv"] = ".csv";
  MimeTypeMap["text/javascript"] = ".js";
  MimeTypeMap["application/javascript"] = ".js";
  MimeTypeMap["image/jpeg"] = ".jpg";
  MimeTypeMap["image/png"] = ".png";
  MimeTypeMap["image/gif"] = ".gif";
  MimeTypeMap["image/svg+xml"] = ".svg";
  MimeTypeMap["image/webp"] = ".webp";
  MimeTypeMap["image/bmp"] = ".bmp";
  MimeTypeMap["audio/mpeg"] = ".mp3";
  MimeTypeMap["audio/wav"] = ".wav";
  MimeTypeMap["audio/ogg"] = ".ogg";
  MimeTypeMap["video/mp4"] = ".mp4";
  MimeTypeMap["video/x-msvideo"] = ".avi";
  MimeTypeMap["video/webm"] = ".webm";
  MimeTypeMap["video/quicktime"] = ".mov";
  MimeTypeMap["video/x-flv"] = ".flv";



  std::string name;
  const char charset[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
  name += "file_";
  for (int i = 0; i < 5; ++i) {
    int index = rand() % (sizeof(charset) - 1);
    name += charset[index];
  }
  std::cerr << "generated name: " << name << std::endl;
  std::cerr << "MimeType: " << MimeType << std::endl;
  if (MimeTypeMap.find(MimeType) != MimeTypeMap.end()) {
    std::cerr << "found mime type: " << MimeType << std::endl;
    return name + MimeTypeMap[MimeType];
  }
  return name + ".bin";
}

static void ParseContentDisposition(client_info& client, std::map<int, server_config>& server) {
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
    close(client.file_fd);
    client.filename.clear();
    client.filename = nameGenerator(client.contentTypeform);
    std::string filename = client.filename;
    client.file_fd = open(filename.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (client.file_fd == -1) {
      std::cerr << "Error opening file" << std::endl;
      return ;//
      error_response(client, server[client.index_server], 500);//is it 500?
    }
    client.data = client.data.substr(2 + (client.pos != 0 && client.bodyTypeTaken != 3) * 2, client.data.size());
  } else if (client.pos != std::string::npos) {
    close(client.file_fd);
    client.data = client.data.substr(client.pos + 10, client.data.size());
    client.filename = client.data.substr(0, client.data.find("\""));
    std::cerr << "filename: " << client.filename << std::endl;
    client.data = client.data.substr(client.data.find("\"") + 3 , client.data.size());
    client.file_fd = open(client.filename.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (client.file_fd == -1) {
      std::cerr << "Error opening file" << std::endl;
      return ;//
      error_response(client, server[client.index_server], 500);//is it 500?
    }
  }
}

static void ParseContentType(client_info& client) {
  client.data = client.data.substr(client.pos + 14, client.data.size());
  client.pos = client.data.find("/");
  client.data = client.data.substr(client.pos + 1, client.data.size());
  client.contentTypeform = client.data.substr(0, client.data.find("\r\n"));
  if (client.bodyTypeTaken == 1 || client.bodyTypeTaken == 2)
    client.data = client.data.substr(client.data.find("\r\n") + 6, client.data.size());
  else
    client.data = client.data.substr(client.data.find("\r\n") + 4, client.data.size());
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
