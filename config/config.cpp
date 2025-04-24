#include "../server.hpp"

void server::parse_config(std::string config_file)
{
	int i = 0;
	std::stack<std::string> stack;
	std::ifstream file(config_file);
	if (!file.good())
		throw std::runtime_error("Config file could not be opened");
	std::string line;
	std::string location_index;
	while (std::getline(file, line))
	{
		std::istringstream ss(line);
		std::string key;
		ss >> key;
		if (line.empty())
			continue;
		else if (line == "server {")
		{
			if (!stack.empty())
				throw std::runtime_error("cannot create server inside server");
			servers[i].server_index = i;
			stack.push("server");
			continue;
		}
		else if (key == "location" && stack.top() == "server")
		{
			ss >> location_index;
			if (location_index.empty())
				throw std::runtime_error("location block not opened");
			if (servers[i].locations.find(location_index) != servers[i].locations.end())
				throw std::runtime_error("Duplicate location");
			std::string close;
			ss >> close;
			if (close != "{")
				throw std::runtime_error("location block not opened");
			stack.push("location");
			servers[i].locations[location_index].location_index = location_index;
			continue;
		}
		else if (key == "}" && somthing_after(ss) == 0)
		{
			if (stack.empty())
				throw std::runtime_error("server block not opened");
			else if (stack.top() == "server" &&
					 (servers[i].host.empty() || servers[i].ports.empty() || servers[i].path.empty() || servers[i].max_body_size == 0))
			{
				throw std::runtime_error("key missing in server block");
			}
			if (stack.top() == "server")
				i++;
			else if (stack.top() == "location" && servers[i].locations[location_index].path.empty())
				servers[i].locations[location_index].path = servers[i].path;
			if (stack.top() == "location" && servers[i].locations[location_index].upload_path.empty())
			{
				servers[i].locations[location_index].upload_path = servers[i].upload_path;
				if (servers[i].locations[location_index].upload_path.empty() && find(servers[i].locations[location_index].allowed_methods.begin(), servers[i].locations[location_index].allowed_methods.end(), "POST") != servers[i].locations[location_index].allowed_methods.end())
					throw std::runtime_error("upload path missing in location block");
			}
			if (stack.top() == "location" && servers[i].locations[location_index].allowed_methods.empty())
				throw std::runtime_error("allowed methods missing in location block");
			stack.pop();
			continue;
		}
		else if (!stack.empty() && stack.top() == "location")
			parse_location(ss, key, servers[i].locations[location_index]);
		else if (!stack.empty() && stack.top() == "server")
			parse_key(ss, key, servers[i]);
		else
			throw std::runtime_error("syntax error");
	}
	if (!stack.empty())
		throw std::runtime_error("server block not closed");
	else if (servers.empty())
		throw std::runtime_error("no server block found");
	file.close();
	std::cout << "------------config file parsed------------" << std::endl;
}
