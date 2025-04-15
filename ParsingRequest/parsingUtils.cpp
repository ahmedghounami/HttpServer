/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   parsingUtils.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hboudar <hboudar@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/15 17:23:29 by hboudar           #+#    #+#             */
/*   Updated: 2025/04/11 16:41:03 by hboudar          ###   ########.fr       */
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
