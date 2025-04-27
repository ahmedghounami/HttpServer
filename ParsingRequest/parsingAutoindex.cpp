
#include "../server.hpp"

bool autoindex_server(client_info &client, server_config &server)
{
	struct stat info;
	if (server.index.empty() == false && stat((server.path + "/" + server.index[0].c_str()).c_str(), &info) == 0 && access((server.path + "/" + server.index[0].c_str()).c_str(), R_OK) == 0)
	{
		if (stat((server.path + "/" + server.index[0].c_str()).c_str(), &info) == 0 && access((server.path + "/" + server.index[0].c_str()).c_str(), R_OK) == 0)
			client.uri = "/" + server.index[0];
		else
		{
			generateAutoindexToFile(client.uri, server.path, server.path + "/direc.html");
			client.uri = "/direc.html";
		}
	}

	else if (server.index.empty() == false && (stat((server.path + "/" + server.index[0].c_str()).c_str(), &info) != 0 || access((server.path + "/" + server.index[0].c_str()).c_str(), R_OK) != 0) && server.autoindex == true)
	{
		generateAutoindexToFile(client.uri, server.path, server.path + "/direc.html");
		client.uri = "/direc.html";
	}
	else if (server.index.empty() == false && (stat((server.path + "/" + server.index[0].c_str()).c_str(), &info) != 0 || access((server.path + "/" + server.index[0].c_str()).c_str(), R_OK) != 0) && server.autoindex == false)
	{
		error_response(client, server, 404); // 404
		return false;						 // respond and clear client;
	}
	if (server.index.empty() == true && stat((server.path + "/index.html").c_str(), &info) == 0 && access((server.path + "/index.html").c_str(), R_OK) == 0)
	{
		client.uri = "/index.html";
		if (stat((server.path + "/index.html").c_str(), &info) == 0 && access((server.path + "/index.html").c_str(), R_OK) == 0)
			client.uri = "/index.html";
		else
		{
			generateAutoindexToFile(client.uri, server.path, server.path + "/direc.html");
			client.uri = "/direc.html";
		}
	}
	if (server.index.empty() == true && (stat((server.path + "/index.html").c_str(), &info) != 0 || access((server.path + "/index.html").c_str(), R_OK) != 0) && server.autoindex == true)
	{
		generateAutoindexToFile(client.uri, server.path, server.path + "/direc.html");
		client.uri = "/direc.html";
	}
	else if (server.index.empty() == true && (stat((server.path + "/index.html").c_str(), &info) != 0 || access((server.path + "/index.html").c_str(), R_OK) != 0) && server.autoindex == false)
	{
		error_response(client, server, 404); // 404
		return false;						 // respond and clear client;
	}
	return true;
}

bool autoindex(client_info &client, location &loc, server_config &server)
{
	struct stat info;
	if (loc.index.empty() == false && stat((loc.path + "/" + loc.index[0].c_str()).c_str(), &info) == 0 && access((loc.path + "/" + loc.index[0].c_str()).c_str(), R_OK) == 0)
	{
		if (stat((loc.path + "/" + loc.index[0].c_str()).c_str(), &info) == 0 && access((loc.path + "/" + loc.index[0].c_str()).c_str(), R_OK) == 0)
			client.uri = "/" + loc.index[0];
		else
		{
			generateAutoindexToFile(client.uri, loc.path, loc.path + "/direc.html");
			client.uri = "/direc.html";
		}
	}

	else if (loc.index.empty() == false && (stat((loc.path + "/" + loc.index[0].c_str()).c_str(), &info) != 0 || access((loc.path + "/" + loc.index[0].c_str()).c_str(), R_OK) != 0) && loc.autoindex == true)
	{
		generateAutoindexToFile(client.uri, loc.path, loc.path + "/direc.html");
		client.uri = "/direc.html";
	}
	else if (loc.index.empty() == false && (stat((loc.path + "/" + loc.index[0].c_str()).c_str(), &info) != 0 || access((loc.path + "/" + loc.index[0].c_str()).c_str(), R_OK) != 0) && loc.autoindex == false)
	{
		error_response(client, server, 404); // 404
		return false;						 // respond and clear client;
	}
	if (loc.index.empty() == true && stat((loc.path + "/index.html").c_str(), &info) == 0 && access((loc.path + "/index.html").c_str(), R_OK) == 0)
	{
		client.uri = "/index.html";
		if (stat((loc.path + "/index.html").c_str(), &info) == 0 && access((loc.path + "/index.html").c_str(), R_OK) == 0)
			client.uri = "/index.html";
		else
		{
			generateAutoindexToFile(client.uri, loc.path, loc.path + "/direc.html");
			client.uri = "/direc.html";
		}
	}
	if (loc.index.empty() == true && (stat((loc.path + "/index.html").c_str(), &info) != 0 || access((loc.path + "/index.html").c_str(), R_OK) != 0) && loc.autoindex == true)
	{
		generateAutoindexToFile(client.uri, loc.path, loc.path + "/direc.html");
		client.uri = "/direc.html";
	}
	else if (loc.index.empty() == true && (stat((loc.path + "/index.html").c_str(), &info) != 0 || access((loc.path + "/index.html").c_str(), R_OK) != 0) && loc.autoindex == false)
	{
		error_response(client, server, 404); // 404
		return false;						 // respond and clear client;
	}
	return true;
}

void generateAutoindexToFile(const std::string &uri, const std::string &directory_path, const std::string &output_file_path)
{
	DIR *dir = opendir(directory_path.c_str());
	if (!dir)
		return;

	std::stringstream html;
	html << "<!DOCTYPE html>\n"
			"<html>\n"
			"<head><meta charset='UTF-8'><title>Directory Listing</title></head>\n"
			"<style>\n"
			"body { font-family: Arial, sans-serif; }\n"
			"table { width: 100%; border-collapse: collapse; }\n"
			"th, td { padding: 8px; text-align: left; border-bottom: 1px solid #ddd; }\n"
			"tr:hover { background-color: #f1f1f1; }\n"
			"a { text-decoration: none; color: #007BFF; }\n"
			"a:hover { text-decoration: underline; }\n"
			"</style>\n"
			"<body>\n"
			"<h1>Directory Listing</h1>\n"
			"<table>\n"
			"<thead><tr><th>Name</th><th>Type</th></tr></thead>\n"
			"<tbody>\n";

	struct dirent *entry;
	while ((entry = readdir(dir)) != NULL)
	{
		std::string name = entry->d_name;
		// std::cout << "name: " << name << std::endl;
		if (name == ".")
			continue;

		std::string full_path = directory_path + "/" + name;
		struct stat file_stat;
		std::string type = "File";

		if (stat(full_path.c_str(), &file_stat) == 0 && S_ISDIR(file_stat.st_mode))
		{
			name += "/";
			type = "Directory";
		}

		html << "<tr><td><a href=\"" << uri << name << "\">" << name << "</a></td><td>" << type << "</td></tr>\n";
	}

	html << "</tbody>\n</table>\n</body>\n</html>\n";
	closedir(dir);

	// Write to output file
	std::ofstream file(output_file_path);
	if (!file.good())
	{
		std::cerr << "Error: Unable to open file for writing: " << output_file_path << std::endl;
		return;
	}

	file << html.str();
	file.close();
}

bool check_autoindex(client_info &client, std::map<int, server_config> &server)
{
	client.index_server = findMatchingServer(client, server);
	int found = 0;
	std::string location = getlocation(client, server[client.index_server]);
	if (location.empty() == false)
	{
		std::cout << "location: " << getlocation(client, server[client.index_server]) << std::endl;

		found = 1;
		if (std::find(server[client.index_server].locations[location].allowed_methods.begin(), server[client.index_server].locations[location].allowed_methods.end(), client.method) == server[client.index_server].locations[location].allowed_methods.end())
		{
			std::cerr << "Error: method: not allowed: " << client.method << std::endl;
			error_response(client, server[client.index_server], 405); // 405
			return false;											  // respond and clear client;
		}
		if (client.method == "GET")
		{
			if (server[client.index_server].locations[location].redirect.first.empty() == true && client.uri == location)
			{
				if (autoindex(client, server[client.index_server].locations[location], server[client.index_server]) == false)
					return false; // respond and clear client;
			}
			else if (location == client.uri && server[client.index_server].locations[location].redirect.first.empty() == false)
			{
				redirect(client, server[client.index_server].locations[location].redirect);
				return false; // respond and clear client;
			}
		}
		else if (client.method == "POST")
			client.upload_path = server[client.index_server].locations[location].upload_path;
	}

	if (found == 0 && client.uri == "/" && client.method == "GET")
	{
		if (autoindex_server(client, server[client.index_server]) == false)
		{
			std::cerr << "Error: Invalid uri: " << client.uri << std::endl;
			return false; // respond and clear client;
		}
	}
	else if (client.method == "POST" && found == 0)
		client.upload_path = server[client.index_server].upload_path;
	if (client.method == "POST" && client.upload_path.empty() == true)
	{
		error_response(client, server[client.index_server], 405); // 405
		return false;											  // respond and clear client;
	}
	std::cout << "upload_path: " << client.upload_path << std::endl;
	return true;
}
