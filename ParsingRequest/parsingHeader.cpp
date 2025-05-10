#include "../server.hpp"


std::string decodeURIComponent(const std::string& encoded) {
    std::ostringstream decoded;
    for (size_t i = 0; i < encoded.length(); ++i) {
        if (encoded[i] == '%' && i + 2 < encoded.length()) {
            char hex[3] = { encoded[i + 1], encoded[i + 2], '\0' };
            if (isxdigit(hex[0]) && isxdigit(hex[1])) {
                decoded << static_cast<char>(std::strtol(hex, NULL, 16));
                i += 2;
            } else {
                decoded << '%'; // leave it if malformed
            }
        } else if (encoded[i] == '+') {
            decoded << ' '; // optional
        } else {
            decoded << encoded[i];
        }
    }
    return decoded.str();
}

// Main function
bool validateAndNormalizePath(client_info &client, std::map<int, server_config> &server) {
    // 1. Decode URI
    std::cerr << "before tracing_uri: " << client.uri << std::endl;
	  tracing_uri(client.uri);
    std::cerr << "after tracing_uri: " << client.uri << std::endl;
    client.uri = decodeURIComponent(client.uri);
    // 2. Extract and store client.query
    // size_t qpos = client.uri.find('?');
    // if (qpos != std::string::npos) {
    //     client.query = client.uri.substr(qpos + 1);
    //     client.uri = client.uri.substr(0, qpos);
    // } else {
    //     client.query.clear();
    // }

    // // 3. Strip fragment (if ever present)
    // size_t fragPos = client.uri.find('#');
    // if (fragPos != std::string::npos)
    //     client.uri = client.uri.substr(0, fragPos);
	client.isCgi =  handlepathinfo(client);

    // 4. Normalize segments
    std::istringstream ss(client.uri);
    std::string segment;
    std::vector<std::string> segments;

    while (std::getline(ss, segment, '/')) {
        if (segment.empty() || segment == ".")
          continue;
        if (segment == "..") {
            std::cerr << "❌ Path contains '..' which is not allowed.\n";
            error_response(client, server[client.index_server], 400);
            return false;
        }
        segments.push_back(segment);
    }

    // 5. Rebuild normalized path
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


bool RequestLine(client_info &client, std::map<int, server_config> &server)
{
	(void)server;
	if (client.method.empty() == false)
		return true;

	client.headersTaken = false;
	size_t pos = client.data.find("\r\n");
	if (pos == std::string::npos) // not enough data
		return false;

	std::string requestLine = client.data.substr(0, pos);
	client.data.erase(0, pos + 2);

	if (requestLine.empty() || requestLine[0] == ' ')
	{
		std::cerr << "ERROR: Request line start with a space" << std::endl;
		error_response(client, server[client.index_server], 400); // 500
		return false;											  
	}

	size_t start = requestLine.find_first_not_of(" ");
	size_t end = requestLine.find_last_not_of(" ");
	if (end != requestLine.size() - 1)
	{
		std::cerr << "ERROR: Request line ends with extra character(s)" << std::endl;
		error_response(client, server[client.index_server], 400); // 500
		return false;											  
	}

	if (start == std::string::npos)
	{
		std::cerr << "ERROR: Empty request line" << std::endl;
		error_response(client, server[client.index_server], 400); // 500
		return false;											  
	}

	requestLine = requestLine.substr(start, end - start + 1);

	size_t firstSP = requestLine.find(' ');
	size_t secondSP = requestLine.find(' ', firstSP + 1);
	size_t thirdSP = requestLine.find(' ', secondSP + 1);

	if (firstSP == 0 || firstSP == std::string::npos || secondSP == std::string::npos || thirdSP != std::string::npos)
	{
		std::cerr << "Error: Malformed request line (Incorrect spaces)" << std::endl;
		error_response(client, server[client.index_server], 400); // 500
		return false;											  
	}

	client.method = requestLine.substr(0, firstSP);
	client.uri = requestLine.substr(firstSP + 1, secondSP - firstSP - 1);
	client.version = requestLine.substr(secondSP + 1);

	if (client.method != "GET" && client.method != "DELETE" && client.method != "POST" && client.method != "PUT" && client.method != "HEAD" && client.method != "CONNECT" && client.method != "OPTIONS" && client.method != "TRACE")
	{
		std::cerr << "Error: method: not allowed: " << client.method << std::endl;
		error_response(client, server[client.index_server], 405); // 405
		return false;											  
	}
	else if (client.method != "GET" && client.method != "POST" && client.method != "DELETE")
	{
		std::cerr << "Error: Method not implemented: " << client.method << std::endl;
		error_response(client, server[client.index_server], 501); // 501
		return false;											  
	}

	if (client.uri.empty() || client.uri[0] != '/')
	{
		std::cerr << "Error: Invalid request-target (URI must start with '/')" << std::endl;
		error_response(client, server[client.index_server], 404); // 404
		return false;											  
	}
	validateAndNormalizePath(client, server);
	std::cerr << "client.uri: " << client.uri << std::endl;

	if (client.version != "HTTP/1.1" || client.version.find(' ') != std::string::npos)
	{
		std::cerr << "Error: Invalid or malformed HTTP version: " << client.version << std::endl;
		error_response(client, server[client.index_server], 505); // 505
		return false;											  
	}

	// std::cerr << "method '" << client.method << "'\nuri '" << client.uri << "'\nversion '" << client.version << "'\n" << std::endl;
	return true;
}

bool ParseHeaders(client_info &client, std::map<int, server_config> &server)
{

	if (client.headersTaken)
		return true;

	size_t pos = client.data.find("\r\n\r\n");
	if (pos == std::string::npos) {
		static int i = 0;
		i++;
		if (i == 2)
			error_response(client, server[client.index_server], 400);
		return false;
	} // not enough data

	std::string headers = client.data.substr(0, pos);
	client.data.erase(0, pos + 4);

	size_t startPos = 0;
	std::string lastKey;
	while (startPos < headers.size())
	{
		size_t endPos = headers.find("\r\n", startPos);
		if (endPos == std::string::npos)
			endPos = headers.size();

		std::string line = headers.substr(startPos, endPos - startPos);
		startPos = endPos + 2;

		if (line.empty())
			continue;

		if (!lastKey.empty() && (line[0] == ' ' || line[0] == '\t'))
		{
			client.headers[lastKey] += " " + trim(line);
			continue;
		}

		size_t delimiterPos = line.find(":");
		if (delimiterPos == std::string::npos)
		{
			std::cerr << "Error: Malformed header (missing ':'): " << line
					  << std::endl;
			error_response(client, server[client.index_server], 400); // 500
			return false;											  
		}

		std::string key = trim(line.substr(0, delimiterPos));
		std::string value = trim(line.substr(delimiterPos + 1));

		if (key.empty() || value.empty())
		{
			std::cerr << "Error: Empty header name or value" << std::endl;
			error_response(client, server[client.index_server], 400); // 500
			return false;											  
		}

		std::transform(key.begin(), key.end(), key.begin(), ::tolower);
		if (!isValidHeaderKey(key))
		{
			std::cerr << "Error: Invalid header name: " << key << std::endl;
			error_response(client, server[client.index_server], 400); // 500
			return false;											  
		}
		if (!isValidHeaderValue(value))
		{
			std::cerr << "Error: Invalid header value: " << value << std::endl;
			error_response(client, server[client.index_server], 400); // 500
			return false;											  
		}
		if (client.headers.find(key) != client.headers.end())
		{
			client.headers[key] += ", " + value;
		}
		else
			client.headers[key] = value;

		lastKey = key;
	}

	if (client.headers.find("host") == client.headers.end())
	{
		std::cerr << "Error: Missing 'Host' header" << std::endl;
		error_response(client, server[client.index_server], 400); // 500
		return false;											  
	}
	if (check_autoindex(client, server) == false)
	{
		std::cerr << "im in hte check_autoindex" << std::endl;
		return false; 
	}

	// std::map<std::string, std::string>::iterator it;
	// for (it = client.headers.begin(); it != client.headers.end(); ++it)
	// 	std::cout << "header-> " << it->first << ": '" << it->second << "'" << std::endl;

	client.chunkData = "";
	client.ReadFlag = true;
	client.bodyTaken = false;
	client.bodyTypeTaken = 0;
	client.FileSize = 0;
	client.headersTaken = true;
	client.file_fd = -42;
	client.isCgi = handlepathinfo(client);

	return true;
}

bool TakeBodyType(client_info& client, std::map<int, server_config>& server) {

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
          error_response(client, server[client.index_server], 400);
          return false;
        }
        client.bodyTypeTaken = 1;// formDataChunked(client);
      } else {
        client.bodyTypeTaken = 2;// otherDataChunked(client);
      }
    } else {
      error_response(client, server[client.index_server], 400);
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
          error_response(client, server[client.index_server], 400);
          return false;
        }
        client.bodyTypeTaken = 3;// formData(client);
		client.boundary = "--" + client.boundary;
      } else {
        client.bodyTypeTaken = 4;// otherData(client);
      }
    } else {
      error_response(client, server[client.index_server], 400);
      return false;
    }
  }
  std::cerr << "client.bodyTypeTaken: " << client.bodyTypeTaken << std::endl;
  return true;
}

void ParseChunk(client_info &client, std::map<int, server_config> &server)
{
	if (RequestLine(client, server) == false || ParseHeaders(client, server) == false)
		return;

	client.isGet = false;
	if (client.method == "GET")
		client.isGet = true;
	if (client.method == "DELETE")
		handleDeleteRequest(client, server);
	else if (client.method == "POST" && !client.bodyTaken)
	{
		if (TakeBodyType(client, server) == false)
			return;
		if (client.bodyTypeTaken == 1)
			ChunkedFormData(client, server);
		else if (client.bodyTypeTaken == 2)
			ChunkedOtherData(client, server);
		else if (client.bodyTypeTaken == 3)
			FormData(client,server);
		else if (client.bodyTypeTaken == 4)
			OtherData(client, server);
		else if (client.bodyTypeTaken == 0) {
			error_response(client, server[client.index_server], 400);
			return;
		}
	}
	if (client.bodyTaken == true)
	{
		if (client.isCgi == true) {
			handleCgi(client, server, client.uri);
			std::cerr << "cgi finished-------------------------------------------" << std::endl;
			return;
		}
		std::string body = "<html><body><h1>File uploaded successfully!</h1></body></html>";
		post_success(client, body);
	}
}

