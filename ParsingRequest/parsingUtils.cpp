/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   parsingUtils.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hboudar <hboudar@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/15 17:23:29 by hboudar           #+#    #+#             */
/*   Updated: 2025/04/16 21:25:58 by hboudar          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../server.hpp"

std::string trim(const std::string &str) {
  size_t first = str.find_first_not_of(" \t");
  if (first == std::string::npos)
    return "";
  size_t last = str.find_last_not_of(" \t");
  return str.substr(first, last - first + 1);
}

bool isMultiValueHeader(const std::string &header) {
  static const char *multiHeader[] = {"set-cookie",          "www-authenticate",
                                      "proxy-authenticate",  "authorization",
                                      "proxy-authorization", "warning"};
  for (size_t i = 0; i < sizeof(multiHeader) / sizeof(multiHeader[0]); ++i) {
    if (header == multiHeader[i])
      return true;
  }

  return false;
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

bool isValidContentLength(const std::string &lengthStr) {
  std::string trlen = trim(lengthStr);
  
  for(size_t i = 0; i < trlen.length(); ++i) {
    if (!std::isdigit(trlen[i]))
      return false;
  }
  return true;
}

static std::string decodeURIComponent(const std::string& encoded) {
    std::ostringstream decoded;
    for (size_t i = 0; i < encoded.length(); ++i) {
        if (encoded[i] == '%' && i + 2 < encoded.length()) {
            char hex[3] = { encoded[i + 1], encoded[i + 2], '\0' };
            if (isxdigit(hex[0]) && isxdigit(hex[1])) {
                decoded << static_cast<char>(std::strtol(hex, nullptr, 16));
                i += 2;
            } else {
                decoded << '%';
            }
        } else if (encoded[i] == '+') {
            decoded << ' ';  // Optional: '+' as space (for query, not path)
        } else {
            decoded << encoded[i];
        }
    }
    return decoded.str();
}

bool parseRequestPath(client_info& client) {
    // Step 0: Decode URI
    client.uri = decodeURIComponent(client.uri);
    // Check if decoding was successful
    if (client.uri.empty()) {
        std::cerr << "❌ Failed to decode URI.\n";
        return false; // response.error = 400;
    }

    // Step 1: Remove query and fragment
    size_t fragPos = client.uri.find('#');
    if (fragPos != std::string::npos)
        client.uri = client.uri.substr(0, fragPos);

    size_t qpos = client.uri.find('?');
    if (qpos != std::string::npos) {
      client.uri = client.uri.substr(0, qpos);
      client.query = client.uri.substr(qpos + 1);
    }

    // Step 2: Split client.uri and validate components
    std::istringstream ss(client.uri);
    std::string segment;
    std::vector<std::string> segments;

    while (std::getline(ss, segment, '/')) {
        if (segment.empty() || segment == ".")
            continue;
        if (segment == "..") {
          std::cerr << "❌ Path contains '..' which is not allowed.\n";
          return false;//response.error = 400;
        }
        segments.push_back(segment);
    }

    // Step 3: Reconstruct cleaned path
    std::ostringstream cleanPath;
    cleanPath << "/";
    for (size_t i = 0; i < segments.size(); ++i) {
        cleanPath << segments[i];
        if (i != segments.size() - 1)
            cleanPath << "/";
    }

    client.uri = cleanPath.str();
    return true;
}

void writeToFile(std::string &body, int fd) {
  if (fd > 0) {
    if (body.empty() || (body.size() == 2 && body == "\r\n"))
      return ;
    write(fd, body.c_str(), body.size());
  }
}

std::string nameGenerator() {
  srand(time(0));
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

void ParseContentDisposition(client_info& client) {
  std::cerr << "-------------ParseContentDisposition------------" << std::endl;
  client.pos = client.data.find("name=\"");
  client.data = client.data.substr(client.pos + 6, client.data.size());
  std::string sub = client.data.substr(0, client.data.find("\""));
  client.data = client.data.substr(client.data.find("\"") + 3 , client.data.size());
  client.name = sub;

  client.pos = client.data.find("filename=\"");
  if (client.pos == std::string::npos || client.pos != 0)
  {
    // close(client.file_fd);
    client.filename = nameGenerator();
    client.file_fd = open(client.filename.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0644);//check if file_fd is valid
    client.data = client.data.substr(2 + (client.pos != 0) * 2, client.data.size());
  } else if (client.pos != std::string::npos) {
    // close(client.file_fd);
    client.data = client.data.substr(client.pos + 10, client.data.size());
    sub = client.data.substr(0, client.data.find("\""));
    client.data = client.data.substr(client.data.find("\"") + 3 , client.data.size());
    client.filename = sub;
    client.file_fd = open(client.filename.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0644);//check if file_fd is valid
  }
  std::cerr << "name: |" << client.name << "|" << std::endl;
  std::cerr << "filename: |" << client.filename << "|" << std::endl;
  std::cerr << "-------------End ParseContentDisposition--------" << std::endl;
}

void ParseContentType(client_info& client) {
  std::cerr << "--------------ParseContentType----------------" << std::endl;
  client.data = client.data.substr(client.pos + 14, client.data.size());
  client.pos = client.data.find("/");
  client.data = client.data.substr(client.pos + 1, client.data.size());
  client.contentTypeform = client.data.substr(0, client.data.find("\n" - 1));
  client.data = client.data.substr(client.data.find("\n") + 5, client.data.size());

  std::cerr << "Content-Type: |" << client.contentTypeform << "|" << std::endl;
  std::cerr << "--------------End ParseContentType-------------" << std::endl;
}


void NewFile(client_info &client) {
  client.chunkData = "", client.bytesLeft = 0, client.chunkSize = 0;// close(client.file_fd);
  client.data = client.data.substr(client.boundary.size() + 2);

  client.pos = client.data.find("Content-Disposition: form-data;");
  if (client.pos != std::string::npos && client.pos == 0) {
    client.data = client.data.substr(client.pos + 32, client.data.size());
    ParseContentDisposition(client);
  }
  client.pos = client.data.find("Content-Type:");
  if (client.pos != std::string::npos && client.pos == 0) {
    ParseContentType(client);
  }
}

void ReadTheData(client_info& client) {
  bool flag = true;
  while (flag)
  {
    if ((client.data.size() == 2 && client.data == "\r\n")
      || (client.data.size() == 4 && client.data == "\r\n\r\n"))
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
      return ;
    }

    client.pos = client.data.find("\r\n");
    std::string ChunkSizeString = client.data.substr(0, client.pos);
    client.data = client.data.substr(client.pos + 2);
    std::istringstream iss(ChunkSizeString);
    client.chunkSize = 0;
    iss >> std::hex >> client.chunkSize;
    if (client.chunkSize + 2 > client.data.size()) {
      client.bytesLeft = client.chunkSize - (client.data.size() - 2);
      client.chunkData = client.data.substr(0, client.data.size() - 2);
      client.data.clear();
    }
    else
    {
      client.chunkData = client.data.substr(0, client.chunkSize);
      client.data = client.data.substr(client.chunkSize + 2);
      client.bytesLeft = 0;
    }
    if (!client.chunkData.empty())
      writeToFile(client.chunkData, client.file_fd);


    client.pos = client.data.find(client.boundary);
    if (client.pos != std::string::npos && client.bytesLeft == 0)//working in here
      NewFile(client);

    client.pos = client.data.find(client.boundary + "--\r\n\r\n0\r\n\r\n");
    if (client.pos != std::string::npos)
    {
      std::cerr << "End boundary found" << std::endl;
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

  }
}
