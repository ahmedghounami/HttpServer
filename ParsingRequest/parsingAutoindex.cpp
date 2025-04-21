
#include "../server.hpp"

bool autoindex_server(client_info &client, server_config &loc)
{
	struct stat info;
	if (loc.index.empty() == false && stat((loc.path + "/" + loc.index[0].c_str()).c_str(), &info) == 0 && access((loc.path + "/" + loc.index[0].c_str()).c_str(), R_OK) == 0)
		client.uri = "/" + loc.index[0];
	else if (loc.index.empty() == false && (stat((loc.path + "/" + loc.index[0].c_str()).c_str(), &info) != 0 || access((loc.path + "/" + loc.index[0].c_str()).c_str(), R_OK) != 0) && loc.autoindex == true)
	{
		generateAutoindexToFile(client.uri, loc.path, "/Users/aghounam/Desktop/www.webserv/www/direc.html");
		client.uri = "/direc.html";
	}
	else if (loc.index.empty() == false && (stat((loc.path + "/" + loc.index[0].c_str()).c_str(), &info) != 0 || access((loc.path + "/" + loc.index[0].c_str()).c_str(), R_OK) != 0) && loc.autoindex == false)
	{
		not_found(client);
		return false; // respond and clear client;
	}
	if (loc.index.empty() == true && stat((loc.path + "/index.html").c_str(), &info) == 0 && access((loc.path + "/index.html").c_str(), R_OK) == 0)
	{
		std::cout << "autoindex_server-------------------------------" << std::endl;
		client.uri = "/index.html";
	}
	if (loc.index.empty() == true && (stat((loc.path + "/index.html").c_str(), &info) != 0 || access((loc.path + "/index.html").c_str(), R_OK) != 0) && loc.autoindex == true)
	{
		generateAutoindexToFile(client.uri, loc.path, "/Users/aghounam/Desktop/www.webserv/www/direc.html");
		client.uri = "/direc.html";
	}
	else if (loc.index.empty() == true && (stat((loc.path + "/index.html").c_str(), &info) != 0 || access((loc.path + "/index.html").c_str(), R_OK) != 0) && loc.autoindex == false)
	{
		not_found(client);
		return false; // respond and clear client;
	}
	return true;
}

bool autoindex(client_info &client, location &loc)
{
		struct stat info;
	if (loc.index.empty() == false && stat((loc.path + "/" + loc.index[0].c_str()).c_str(), &info) == 0 && access((loc.path + "/" + loc.index[0].c_str()).c_str(), R_OK) == 0)
		client.uri = "/" + loc.index[0];
	else if (loc.index.empty() == false && (stat((loc.path + "/" + loc.index[0].c_str()).c_str(), &info) != 0 || access((loc.path + "/" + loc.index[0].c_str()).c_str(), R_OK) != 0) && loc.autoindex == true)
	{
		generateAutoindexToFile(client.uri, loc.path, "/Users/aghounam/Desktop/www.webserv/www/direc.html");
		client.uri = "/direc.html";
	}
	else if (loc.index.empty() == false && (stat((loc.path + "/" + loc.index[0].c_str()).c_str(), &info) != 0 || access((loc.path + "/" + loc.index[0].c_str()).c_str(), R_OK) != 0) && loc.autoindex == false)
	{
		not_found(client);
		return false; // respond and clear client;
	}
	if (loc.index.empty() == true && stat((loc.path + "/index.html").c_str(), &info) == 0 && access((loc.path + "/index.html").c_str(), R_OK) == 0)
	{
		client.uri = "/index.html";
	}
	if (loc.index.empty() == true && (stat((loc.path + "/index.html").c_str(), &info) != 0 || access((loc.path + "/index.html").c_str(), R_OK) != 0) && loc.autoindex == true)
	{
		generateAutoindexToFile(client.uri, loc.path, "/Users/aghounam/Desktop/www.webserv/www/direc.html");
		client.uri = "/direc.html";
	}
	else if (loc.index.empty() == true && (stat((loc.path + "/index.html").c_str(), &info) != 0 || access((loc.path + "/index.html").c_str(), R_OK) != 0) && loc.autoindex == false)
	{
		not_found(client);
		return false; // respond and clear client;
	}
	return true;
}

void generateAutoindexToFile(const std::string &uri, const std::string &directory_path, const std::string &output_file_path)
{
	DIR *dir = opendir(directory_path.c_str());
	if (!dir)
		return ;

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
		std::cout << "name: " << name << std::endl;
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
