#include "../server.hpp"

bool request_line(client_info &client) {
  if (client.method.empty() == false)
    return true;

  client.headersTaken = 0;
  size_t pos = client.chunk.find("\r\n");
  if (pos == std::string::npos)
    return false;

  std::string requestLine = client.chunk.substr(0, pos);
  client.chunk.erase(0, pos + 2);

  if (requestLine.empty() || requestLine[0] == ' ') {
    std::cerr << "ERROR: Request line start with a space" << std::endl;
    return false; //respond and clear;
  }

  size_t start = requestLine.find_first_not_of(" ");
  size_t end = requestLine.find_last_not_of(" ");
  if (end != requestLine.size() - 1) {
    std::cerr << "ERROR: Request line ends with extra character(s)" << std::endl;
    return false;//respond and clear;
  }
 
  if (start == std::string::npos) {
    std::cerr << "ERROR: Empty request line" << std::endl;
    return false;//respond and clear;
  }

  requestLine = requestLine.substr(start, end - start + 1);

  size_t firstSP = requestLine.find(' ');
  size_t secondSP = requestLine.find(' ', firstSP + 1);
  size_t thirdSP = requestLine.find(' ', secondSP + 1);

  if (firstSP == 0 || firstSP == std::string::npos || secondSP == std::string::npos || thirdSP != std::string::npos) {
    std::cerr << "Error: Malformed request line (Incorrect spaces)" << std::endl;
    return false; //respond and clear client;
  }

  client.method = requestLine.substr(0, firstSP);
  client.uri = requestLine.substr(firstSP + 1, secondSP - firstSP - 1);
  client.version = requestLine.substr(secondSP + 1);

  if (client.method != "GET" && client.method != "DELETE" && client.method != "POST" && client.method != "PUT"
      && client.method != "HEAD" && client.method != "CONNECT" && client.method != "OPTIONS" && client.method != "TRACE") {
    not_allowed_method(client);
    return false; // respond then clear client;
  } else if (client.method != "GET" && client.method != "POST" && client.method != "DELETE") {
    not_implemented_method(client);
    return false; // respond then clear client;
  } else {
    // client.poll_status = 1;
    // client.response = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nContent-Length: ";
    // std::string body = "<html><body>200 OK</body></html>";
    // client.response += std::to_string(body.size()) + "\r\n\r\n" + body;
    // return false; // respond then clear client;
  }

  if (client.uri.empty() || client.uri[0] != '/') {
    std::cerr << "Error: Invalid request-target (URI must start with '/')" << std::endl;
    return false; // respond then clear client;
  }

  if (client.version != "HTTP/1.1" || client.version.find(' ') != std::string::npos) {
     std::cerr << "Error: Invalid or malformed HTTP version: " << client.version << std::endl;
    return false; // respond then clear client;
  }
  std::cerr << "method '" << client.method << "'\nuri '" << client.uri << "'\nversion '" << client.version << "'\n" << std::endl;
  return true;
}

bool headers(client_info &client) {

  if (client.method.empty() ||  client.headersTaken) {
    return true;
  }

  size_t pos = client.chunk.find("\r\n\r\n");
  if (pos == std::string::npos)
    return true;

  std::string headers = client.chunk.substr(0, pos);
  client.chunk.erase(0, pos + 4);

  size_t startPos = 0;
  std::string lastKey;
  while (startPos < headers.size()) {
    size_t endPos = headers.find("\r\n", startPos);
    if (endPos == std::string::npos)
      endPos = headers.size();

    std::string line = headers.substr(startPos, endPos - startPos);
    startPos = endPos + 2;

    if (line.empty())
      continue;

    if (!lastKey.empty() && (line[0] == ' ' || line[0] == '\t')) {
      client.headers[lastKey] += " " + trim(line);
      continue;
    }

    size_t delimiterPos = line.find(":");
    if (delimiterPos == std::string::npos) {
      std::cerr << "Error: Malformed header (missing ':'): " << line
                << std::endl;
      exit (1);
      return false; // respond and clear client;
    }

    std::string key = trim(line.substr(0, delimiterPos));
    std::string value = trim(line.substr(delimiterPos + 1));

    if (key.empty() || value.empty()) {
      std::cerr << "Error: Empty header name or value" << std::endl;
      exit (1);
      return false; // respond and clear client;
    }

    std::transform(key.begin(), key.end(), key.begin(), ::tolower);
    if (!isValidHeaderKey(key)) {
      std::cerr << "Error: Invalid header name: " << key << std::endl;
      exit (1);
      return false; // respond and clear client;
    }
    if (!isValidHeaderValue(value)) {
      std::cerr << "Error: Invalid header value: " << value << std::endl;
      exit (1);
      return false; // respond and clear client;
    }
    if (client.headers.find(key) != client.headers.end())
      client.headers[key] += ", " + value;
    else
      client.headers[key] = value;
      
    lastKey = key;
  }

  if (client.headers.find("host") == client.headers.end()) {
    std::cerr << "Error: Missing 'Host' header" << std::endl;
    exit (1);
    return false; // respond and clear client;
  }

  std::map<std::string, std::string>::iterator it;
  for (it = client.headers.begin(); it != client.headers.end(); ++it) {
    std::cout << "header-> " << it->first << ": '" << it->second << "'" << std::endl;
  }

  client.headersTaken = 1;
  client.bodyTypeTaken = 0;
  client.contentLength = 0;
  client.isChunked = false;

  return true;
}

void parse_chunk(client_info &client, std::map<int, server_config> &server) {
  if (request_line(client) == false || headers(client) == false)
    return ;
  if (client.method == "GET" || client.method == "DELETE") {
    //respond and clear client;
    return ;
  } else if (!takeBody(client))
    return ;

  (void)server;
  (void)client;
}
