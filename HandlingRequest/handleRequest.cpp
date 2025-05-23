
#include "../server.hpp"

std::string getContentType(const std::string &path)
{
    std::string extension = path.substr(path.find_last_of(".") + 1);
    if (extension == "html" || extension == "htm")
        return "text/html";
    else if (extension == "css")
        return "text/css";
    else if (extension == "js")
        return "application/javascript";
    else if (extension == "json")
        return "application/json";
    else if (extension == "png")
        return "image/png";
    else if (extension == "jpg" || extension == "jpeg")
        return "image/jpeg";
    else if (extension == "pdf")
        return "application/pdf";
    else if (extension == "txt")
        return "text/plain";
    else if (extension == "mp4")
        return "video/mp4";
    else if (extension == "mp3")
        return "audio/mpeg";
    else if (extension == "xml")
        return "application/xml";
    else if (extension == "gif")
        return "image/gif";
    else if (extension == "svg")
        return "image/svg+xml";
    else if(extension == "ico")
        return "image/x-icon";
    return "text/html";
}

int findMatchingServer(client_info &client, std::map<int, server_config> &server)
{
    std::string host;
    int port;
    int server_index = -1;
    for (std::map<std::string, std::string>::iterator it = client.headers.begin(); it != client.headers.end(); ++it)
    {
        if (it->first == "host")
        {
            std::istringstream iss(it->second);
            std::getline(iss, host, ':');
            iss >> port;
            for (std::map<int, server_config>::iterator it = server.begin(); it != server.end(); ++it)
            {
                if(client.ip == it->second.host || (client.ip == "127.0.0.1" && (it->second.host == "localhost" || it->second.host == "127.0.0.1")))
                {
                    if (std::find(it->second.ports.begin(), it->second.ports.end(), port) != it->second.ports.end() && server_index == -1)
                        server_index = it->second.server_index;
                    if (std::find(it->second.server_names.begin(), it->second.server_names.end(), host) != it->second.server_names.end() && std::find(it->second.ports.begin(), it->second.ports.end(), port) != it->second.ports.end())
                    {
                        server_index = it->second.server_index;
                        break;
                    }
                }
            }
        }
    }
    return server_index;
}
void success(client_info &client, std::string body, bool whith_header, std::string path ="", std::string content_type = "", long long file_size = -1)
{
    if( content_type == "")
    {
        content_type = "Content-Type: " + getContentType(path);
    } 
    client.poll_status = 1;
    if (whith_header)
    {
        if(file_size == -1)
        {
            std::ifstream file(path.c_str());
            if (!file.is_open())
            {
                std::cerr << "Error opening file: " << path << std::endl;
                client.error_code = 500;
                return;
            }
            file.seekg(0, std::ios::end);
            file_size = file.tellg();
            file.seekg(0, std::ios::beg);
        }
        std::string conection = client.headers["connection"];
        if (conection.empty())
            conection = "keep-alive";
        client.response = "HTTP/1.1 200 OK\r\n";
        client.response += content_type + "\r\n";
        client.response += "Content-Length: " + to_string_custom(file_size) + "\r\n";
        client.response += "Connection: " + conection + "\r\n";
        client.response += "\r\n";
        client.bytes_sent = ((double)client.response.size()  * -1) - 1;
    }
    else
        client.response += body;
    
}
std::string getlocation(client_info &client, server_config &server)
{
    std::string uri = client.uri;
    while (uri.size() > 0)
    {
        size_t pos = uri.find_last_of("/");
        if (pos == std::string::npos)
            break;
        if (pos == 0 && uri.size() > 1)
            pos++;
        std::map<std::string, location>::iterator it = server.locations.find(uri);
        if (it != server.locations.end())
            return it->first;
        uri = uri.substr(0, pos);
    }

    return "";
}
std::string getcorectserver_path(client_info &client, std::map<int, server_config> &server)
{
    int server_index = findMatchingServer(client, server);
    std::string loc = client.location;
    if (loc != "" && server[server_index].locations[loc].path != "")
        return server[server_index].locations[loc].path;
    return server[server_index].path;
}
std::string readheadercgi(int fd, std::string &body)
{
    char buffer[1024];
    int bytes_read;
    std::string headers; 
    while ((bytes_read = read(fd, buffer, sizeof(buffer))) > 0)
    {
        headers += std::string(buffer, bytes_read);
        if (headers.find("\r\n\r\n") != std::string::npos)
        {
            size_t pos = headers.find("\r\n\r\n");
            size_t size = headers.size() - pos - 4;
            body = headers.substr(pos + 4, size);
            headers = headers.substr(0, pos + 4);
            break;
        }
    }
    return headers;
}
void cgienv(std::vector<std::string> &envStrings, client_info &client, std::map<int, server_config> &server)
{

    envStrings.push_back("REQUEST_METHOD=" + client.method);
    if(client.headers["authorization"].find("Basic") != std::string::npos)
        envStrings.push_back("AUTH_TYPE=Basic");
    else if (client.headers["authorization"].find("Digest") != std::string::npos)
        envStrings.push_back("AUTH_TYPE=Digest");
    else
        envStrings.push_back("AUTH_TYPE=");
    envStrings.push_back("SCRIPT_NAME=" + client.uri);
    envStrings.push_back("PATH_INFO=" + client.path_info);
    envStrings.push_back("QUERY_STRING=" + client.query);
    envStrings.push_back("CONTENT_TYPE=" + client.ContentType);
    envStrings.push_back("CONTENT_LENGTH=" + client.headers["content-length"]);
    envStrings.push_back("SERVER_NAME=" + client.headers["host"].substr(0, client.headers["host"].find(":")));
    envStrings.push_back("SERVER_PORT=" + client.headers["host"].substr(client.headers["host"].find(":") + 1));
    envStrings.push_back("SERVER_PROTOCOL=" + client.version);
    envStrings.push_back("GATEWAY_INTERFACE=CGI/1.1");
    envStrings.push_back("REMOTE_ADDR=" + client.headers["host"].substr(0, client.headers["host"].find(":")));
    envStrings.push_back("REMOTE_HOST=" + client.headers["host"].substr(0, client.headers["host"].find(":")));
    envStrings.push_back("REMOTE_IDENT=" + client.headers["host"].substr(0, client.headers["host"].find(":")));
    envStrings.push_back("REMOTE_USER=" + client.headers["host"].substr(0, client.headers["host"].find(":")));
    envStrings.push_back("SERVER_SOFTWARE=webserv/1.0");
    envStrings.push_back("PATH_TRANSLATED=" + getcorectserver_path(client, server) + client.uri);
    for (std::map<std::string, std::string>::iterator it = client.headers.begin(); it != client.headers.end(); ++it)
    {
        std::string header = it->first;
        std::transform(header.begin(), header.end(), header.begin(), ::toupper);
        envStrings.push_back("HTTP_" + header + "=" + it->second);
    }
    envStrings.push_back("REDIRECT_STATUS=200");
}
bool checkfiles(client_info &client,  std::map<int, server_config> &server, int &fdin, int &fd, bool &already)
{
    char filename[] = "/tmp/cgi_outputXXXXXX";
    std::string location = client.location;
    if(location != "")
    {
        std::vector<std::string>::iterator start = server[client.index_server].locations[location].cgi_extension.begin();
        std::vector<std::string>::iterator end = server[client.index_server].locations[location].cgi_extension.end();
        if(std::find(start, end, client.uri.substr(client.uri.find_last_of("."))) == end)
        {
            error_response(client, server[client.index_server], 405);
            return true;
        }
    }
    if(!client.isGet && client.method.find("POST") != std::string::npos)
    {
        fdin = open(client.post_cgi_filename.c_str(), O_RDWR , 0666);
        if (fdin == -1)
        {
            std::cerr << "open failed" << std::endl;
            error_response(client, server[client.index_server], 500); // 500
            return true;
        }
    }
    client.isGet = 1;
    if(client.cgi_output != "")
    {
        fd = open(client.cgi_output.c_str(), O_RDWR);
        if (fd == -1)
        {
            std::cerr << "open failed" << std::endl;
            error_response(client, server[client.index_server], 500); // 500
            close(fdin);
            std::remove(client.post_cgi_filename.c_str());
            client.post_cgi_filename = "";
            std::remove(client.cgi_output.c_str());
            client.cgi_output = "";
            return true;
        }
        already = true;
    }
    else
    {
        fd = mkstemp(filename);// create a temporary file whit unique name
        if (fd == -1)
        {
            std::cerr << "mkstemp failed" << std::endl;
            error_response(client, server[client.index_server], 500); // 500
            close(fdin);
            std::remove(client.post_cgi_filename.c_str());
            client.post_cgi_filename = "";
            return true;
        }
        client.cgi_output = filename;
    }
    return false;
}
void close_cgi_in_out(client_info &client, int &fdin, int &fd)
{
    close(fd);
    close(fdin);
    std::remove(client.cgi_output.c_str());
    client.cgi_output = "";
    std::remove(client.post_cgi_filename.c_str());
    client.post_cgi_filename = "";
}
void child_process(client_info &client, int &fd, int &fdin, std::map<int, server_config> &server, std::string &path, bool already)
{
    std::string location = client.location;
    if(already)
    {
        exit(0);
    }
    dup2(fd, STDOUT_FILENO);
    close(fd);
    std::vector<std::string> envStrings;
    cgienv(envStrings, client, server);
    if (client.method.find("POST") != std::string::npos)
    {
        dup2(fdin, STDIN_FILENO);
        close(fdin);
    }
    
    char* envp[envStrings.size() + 1];
    size_t i = 0;
    for (std::vector<std::string>::iterator it = envStrings.begin(); it != envStrings.end(); ++it)
    {
        envp[i] = (char *)it->c_str();
        i++;
    }
    envp[i] = NULL;
    char *cgi_path;
    if (path.substr(path.find_last_of(".") + 1) == "php" && server[client.index_server].locations[location].cgi_path_php != "")
        cgi_path = (char *)server[client.index_server].locations[location].cgi_path_php.c_str();
    else if (path.substr(path.find_last_of(".") + 1) == "py" && server[client.index_server].locations[location].cgi_path_py != "")
        cgi_path = (char *)server[client.index_server].locations[location].cgi_path_py.c_str();
    else
        exit(12);
    char *args[] = {cgi_path, (char *)path.c_str(), NULL};
    if(server[client.index_server].locations[location].cgi_timeout > 0)
        alarm(server[client.index_server].locations[location].cgi_timeout);
    else
        alarm(5);
    if(execve(args[0], args, envp) == -1){
        std::cerr << "execve failed" << std::endl;
        exit(1);}
    exit(0);
}
void handleCgi(client_info &client, std::map<int, server_config> &server)
{
    int fd;
    std::string body;
    std::string content_type = "";
    bool already = false;
    int fdin;
    std::string path = getcorectserver_path(client, server) + client.uri;
    if( checkfiles(client, server, fdin, fd, already))
        return;
    if (client.pid == 0)
        client.pid = fork();
    if (client.pid == -1)
    {
        std::cerr << "fork failed" << std::endl;
        error_response(client, server[client.index_server], 500); // 500
        close_cgi_in_out(client, fdin, fd);
        return;
    }
    else if (client.pid == 0)
    {
        child_process(client, fd, fdin, server, path, already);
    }
    else
    {
        int status;
        int ret = waitpid(client.pid, &status, WNOHANG);
        if(ret == 0){
            close(fd);
            close(fdin);
            return;}
        alarm(0);
        if (WIFEXITED(status))
        {
            int exitstatus = WEXITSTATUS(status);
            if (exitstatus != 0)
            {
                if(exitstatus == 12)
                {
                    std::cerr << "CGI path not found" << std::endl;
                    error_response(client, server[client.index_server], 404); // 404
                    close_cgi_in_out(client, fdin, fd);
                    return;
                }
                std::cerr << "CGI process exited with status: " << exitstatus << std::endl;
                error_response(client, server[client.index_server], 500); // 500
                close_cgi_in_out(client, fdin, fd);
                return;
            }
        }else if (WIFSIGNALED(status))
        {
            std::cerr << "CGI process killed by signal: " << WTERMSIG(status) << std::endl;
            int signal = WTERMSIG(status);
            if(signal == SIGALRM)
                error_response(client, server[client.index_server], 504); // 504
            else
                error_response(client, server[client.index_server], 500); // 500
            close_cgi_in_out(client, fdin, fd);
            return;
        }
        lseek(fd, 0, SEEK_SET);
        char buffer[1024];
        int bytes_read;
        std::string headers = readheadercgi(fd, body);
        if(headers.find("\r\n\r\n") == std::string::npos)
        {
            headers.clear();
            lseek(fd, 0, SEEK_SET);
        }
        if(client.bytes_sent <= 0 && client.bytes_sent != -1 )
        {
            while ((bytes_read = read(fd, buffer, sizeof(buffer))) > 0)
                body+= std::string(buffer, bytes_read);
            if(!headers.empty())
                content_type = headers.erase(headers.find("\r\n\r\n"), 4);
            close(fd);
            close(fdin);
            success(client, body, true, path, content_type, body.size());
            return;
        }
        else if(client.bytes_sent == -1)
            client.bytes_sent = 0;
        while  (body.size() < client.bytes_sent && (bytes_read = read(fd, buffer, sizeof(buffer))) > 0 )
            body += std::string(buffer, bytes_read);
        if(body.size() >= client.bytes_sent)
        {
            std::string temp = body.substr(client.bytes_sent, body.size() - client.bytes_sent);
            body = temp;
        }
        while (body.size() < READ_BUFFER_SIZE && (bytes_read = read(fd, buffer, sizeof(buffer))) > 0)
            body += std::string(buffer, bytes_read);
        if(read(fd, buffer, sizeof(buffer)) == 0)
        {
            client.datafinished = 1;
            close_cgi_in_out(client, fdin, fd);
            client.cgi_output = "";
        }
        close(fd);
        close(fdin);
        success(client, body, false, path, content_type);
        return;
    }
    close(fd);
    close(fdin);
}
void sendbodypart(client_info &client, std::string path)
{
    std::string body;
    if(client.bytes_sent == -1)
        client.bytes_sent = 0;
    std::ifstream file(path.c_str());
    char buffer[READ_BUFFER_SIZE];
    file.seekg(0, std::ios::beg);                 // move to the beginning
    file.seekg(client.bytes_sent, std::ios::beg); // move to byte  from the beginning
    file.read(buffer, READ_BUFFER_SIZE);
    if (file.eof())
    {
        client.datafinished = 1;
    }
    body = std::string(buffer, file.gcount());
    success(client, body, false);
    file.close();
}
bool handlepathinfo(client_info &client){
    bool is_php = false;
    bool is_cgi = false;
    size_t pos = client.uri.find(".php");
    if (pos == std::string::npos)
        pos = client.uri.find(".py");
    else{
        is_php = true;
    }
    if (pos == std::string::npos)
    {
        is_cgi = false;
        return false;
    }
    else
        is_cgi = true;
    std::string path = client.Path + client.uri.substr(0, pos + 3 + is_php);
    struct stat info;
    while (stat(path.c_str(), &info) == 0)
    {
        if((info.st_mode & S_IFDIR) == 0)
            break;
        if(client.uri.find(".php", pos + 3 + is_php) != std::string::npos)
            pos = client.uri.find(".php", pos + 3 + is_php), is_php = true;
        else
        {
            pos = client.uri.find(".py", pos + 3 + is_php);
            if (pos == std::string::npos){
                is_cgi = false;
                return false;}
            else
                is_php = false;
        }
        path = client.Path + client.uri.substr(0, pos + 3 + is_php);
    }
    if (pos == std::string::npos)
        pos = client.uri.find(".py");
    else
        is_php = true;
    if (pos != std::string::npos && (client.uri[pos + is_php + 3] == '/'))
    {
        int add = 3 + is_php;
        std::string path_info = client.uri.substr(pos + add);
        client.path_info = path_info;
        client.uri = client.uri.substr(0, pos + add);
    }
    size_t dot = client.uri.find_last_of(".");
    if (dot != std::string::npos)
    {
        std::string ext = client.uri.substr(dot + 1);
        if (ext == "php" || ext == "py")
            is_cgi = true;
    }
    return is_cgi;
}
void handleGetRequest(client_info &client, std::map<int, server_config> &server)
{
    if(client.error_code != 0)
    {
        error_response(client, server[client.index_server], client.error_code); // 500
        return;
    }
    client.response.clear();
    std::string content_type = "";
    long content_size = -1;
    handlepathinfo(client);
    std::string path = getcorectserver_path(client, server) + client.uri;
    std::ifstream file(path.c_str());
    if (file.fail() || !file.is_open())
    {
        std::cout << "errorcode: " << errno << std::endl;
        switch (errno)
        {
        case ENOENT:
            error_response(client, server[client.index_server], 404);
            break;
        case EACCES:
            error_response(client, server[client.index_server], 403);
            break;
        case ENOTDIR:
            error_response(client, server[client.index_server], 404);
            break;
        case EISDIR:
            error_response(client, server[client.index_server], 403);
            break;
        default:
            error_response(client, server[client.index_server], 500);
            break;
        }
        return;
    }
    if(path.find_last_of(".") != std::string::npos)
    {
        if(path.substr(path.find_last_of(".") + 1) == "php" || path.substr(path.find_last_of(".") + 1) == "py")
        {
            handleCgi(client, server);
            return;
        }
    }
    if(client.bytes_sent <= 0 && client.bytes_sent != -1 )
    {
        success(client, "", true, path, content_type, content_size);
        return;
    }
    sendbodypart(client, path);
    file.close();

}

void handleDeleteRequest(client_info &client, std::map<int, server_config> &server)
{
    if(client.error_code != 0)
    {
        error_response(client, server[client.index_server], client.error_code); // 500
        return;
    }
    std::string path = getcorectserver_path(client, server) + client.uri;
    if (std::remove(path.c_str()) != 0)
    {
        std::cerr << "Error deleting file: " << path << std::endl;
        switch (errno)
        {
        case ENOENT:
            error_response(client, server[client.index_server], 404); // 404
            break;
        case EACCES:
            error_response(client, server[client.index_server], 403); // 403
            break;
        default:
            error_response(client, server[client.index_server], 500); // 500
            break;
        }
    }
    else
    {
        std::string body = "File deleted successfully";
        client.response = "HTTP/1.1 200 OK\r\n";
        client.response += "Content-Type: text/plain\r\n";
        client.response += "Content-Length: " + to_string_custom(body.size()) + "\r\n";
        client.response += "Connection: keep-alive\r\n";
        client.response += "\r\n";
        client.response += body;
        client.poll_status = 1;
        client.datafinished = 1;
    }
}

