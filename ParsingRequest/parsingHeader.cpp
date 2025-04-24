#include "../server.hpp"

bool request_line(client_info &client, std::map<int, server_config> &server)
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
		return false;											  // respond and clear client;
	}

	size_t start = requestLine.find_first_not_of(" ");
	size_t end = requestLine.find_last_not_of(" ");
	if (end != requestLine.size() - 1)
	{
		std::cerr << "ERROR: Request line ends with extra character(s)" << std::endl;
		error_response(client, server[client.index_server], 400); // 500
		return false;											  // respond and clear client;
	}

	if (start == std::string::npos)
	{
		std::cerr << "ERROR: Empty request line" << std::endl;
		error_response(client, server[client.index_server], 400); // 500
		return false;											  // respond and clear client;
	}

	requestLine = requestLine.substr(start, end - start + 1);

	size_t firstSP = requestLine.find(' ');
	size_t secondSP = requestLine.find(' ', firstSP + 1);
	size_t thirdSP = requestLine.find(' ', secondSP + 1);

	if (firstSP == 0 || firstSP == std::string::npos || secondSP == std::string::npos || thirdSP != std::string::npos)
	{
		std::cerr << "Error: Malformed request line (Incorrect spaces)" << std::endl;
		error_response(client, server[client.index_server], 400); // 500
		return false;											  // respond and clear client;
	}

	client.method = requestLine.substr(0, firstSP);
	client.uri = requestLine.substr(firstSP + 1, secondSP - firstSP - 1);
	client.version = requestLine.substr(secondSP + 1);

	if (client.method != "GET" && client.method != "DELETE" && client.method != "POST" && client.method != "PUT" && client.method != "HEAD" && client.method != "CONNECT" && client.method != "OPTIONS" && client.method != "TRACE")
	{
		std::cerr << "Error: method: not allowed: " << client.method << std::endl;
		error_response(client, server[client.index_server], 405); // 405
		return false;											  // respond and clear client;
	}
	else if (client.method != "GET" && client.method != "POST" && client.method != "DELETE")
	{
		std::cerr << "Error: Method not implemented: " << client.method << std::endl;
		error_response(client, server[client.index_server], 501); // 501
		return false;											  // respond and clear client;
	}

	if (client.uri.empty() || client.uri[0] != '/')
	{
		std::cerr << "Error: Invalid request-target (URI must start with '/')" << std::endl;
		error_response(client, server[client.index_server], 404); // 404
		return false;											  // respond and clear client;
	}

	if (client.version != "HTTP/1.1" || client.version.find(' ') != std::string::npos)
	{
		std::cerr << "Error: Invalid or malformed HTTP version: " << client.version << std::endl;
		error_response(client, server[client.index_server], 505); // 505
		return false;											  // respond and clear client;
	}

	// std::cerr << "method '" << client.method << "'\nuri '" << client.uri << "'\nversion '" << client.version << "'\n" << std::endl;
	return true;
}

bool headers(client_info &client, std::map<int, server_config> &server)
{

	if (client.headersTaken)
		return true;

	size_t pos = client.data.find("\r\n\r\n");
	if (pos == std::string::npos) // not enough data
		return false;

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
			return false;											  // respond and clear client;
		}

		std::string key = trim(line.substr(0, delimiterPos));
		std::string value = trim(line.substr(delimiterPos + 1));

		if (key.empty() || value.empty())
		{
			std::cerr << "Error: Empty header name or value" << std::endl;
			error_response(client, server[client.index_server], 400); // 500
			return false;											  // respond and clear client;
		}

		std::transform(key.begin(), key.end(), key.begin(), ::tolower);
		if (!isValidHeaderKey(key))
		{
			std::cerr << "Error: Invalid header name: " << key << std::endl;
			error_response(client, server[client.index_server], 400); // 500
			return false;											  // respond and clear client;
		}
		if (!isValidHeaderValue(value))
		{
			std::cerr << "Error: Invalid header value: " << value << std::endl;
			error_response(client, server[client.index_server], 400); // 500
			return false;											  // respond and clear client;
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
		return false;											  // respond and clear client;
	}
	// else
	// {
	// 	std::string host;
	// 	std::string port;
	// 	int found = 0;
	// 	// split host by ':'
	// 	size_t pos = client.headers["host"].find(":");
	// 	if (pos != std::string::npos)
	// 	{
	// 		host = client.headers["host"].substr(0, pos);
	// 		port = client.headers["host"].substr(pos + 1);
	// 		for (unsigned int i = 0; i < server.size(); i++)
	// 		{
	// 			if (server[i].host == host && find(server[i].ports.begin(), server[i].ports.end(), std::atof(port.c_str())) != server[i].ports.end())
	// 			{
	// 				found = 1;
	// 				break;
	// 			}
	// 		}
	// 		if (found == 0)
	// 		{
	// 			std::cerr << "Error: Invalid host: " << host << ":" << port << std::endl;
	// 			error_response(client, server[client.index_server], 404, ""); // 404 not found
	// 			return false; // respond and clear client;
	// 		}
	// 	}
	// 	else
	// 	{
	// 		std::cerr << "Error: Invalid host: " << client.headers["host"] << std::endl;
	// 		error_response(client, server[client.index_server], 400, ""); // 500
	// 		return false; // respond and clear client;
	// 	}
	// }
	if (check_autoindex(client, server) == false)
	{
		std::cerr << "im in hte check_autoindex" << std::endl;
		return false; // respond and clear client;
	}

	// std::map<std::string, std::string>::iterator it;
	// for (it = client.headers.begin(); it != client.headers.end(); ++it)
	// 	std::cout << "header-> " << it->first << ": '" << it->second << "'" << std::endl;

	client.chunkData = "";
	client.ReadFlag = true;
	client.bodyTaken = false;
	client.bodyTypeTaken = 0;
	client.headersTaken = true;
	client.file_fd = -42;

	// check if the method is post but is a cgi request to make the data store a file and forward it to the cgi
	return true;
}

void parse_chunk(client_info &client, std::map<int, server_config> &server)
{

	// int fd = open("data", O_WRONLY | O_APPEND);//append
	// write(fd, client.data.c_str(), client.data.size());
	// client.file_fd = open("data", O_WRONLY | O_APPEND); // append

	// int fd = open("data1", O_WRONLY | O_APPEND);//append
	// return ;
	if (request_line(client, server) == false || headers(client, server) == false)
		return;
	if (client.method == "POST")
		std::cerr << "------------------------------------the method is " << client.method << std::endl;

	client.isGet = false;
	if (client.method == "GET")
		client.isGet = true;
	else
		client.isGet = false;
	// handleGetRequest(client, server);
	if (client.method == "DELETE")
		handleDeleteRequest(client, server);
	else if (client.method == "POST" && !client.bodyTaken)
	{
		if (takeBodyType(client, server[client.index_server]) == false)
			return;
		if (client.bodyTypeTaken == 1)
			ChunkedFormData(client);
		else if (client.bodyTypeTaken == 2)
			ChunkedOtherData(client);
		else if (client.bodyTypeTaken == 3)
			FormData(client);
	}
	if (client.bodyTaken == true)
	{
		std::string body = "<html><body><h1>File uploaded successfully!</h1></body></html>";
		post_success(client, body);
		std::cerr << "data finished-------------------------------------------" << std::endl;
	}
}
/*notes
	set file descriptor to non-blocking mode
	check the content length in config file with the content length in header
*/
