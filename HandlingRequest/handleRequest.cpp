#include "../server.hpp"
// Caching Considerations:
//  Because GET requests are cacheable, the caching behavior is defined
//  in related RFCs (e.g., RFC 7234). This means that responses to GET
//  requests can be stored and reused under the right conditions.

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
    return "text/html"; // default to HTML if unknown
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
                if (std::find(it->second.ports.begin(), it->second.ports.end(), port) != it->second.ports.end() && server_index == -1)
                    server_index = it->second.server_index;
                if (it->second.host == host && std::find(it->second.server_names.begin(), it->second.server_names.end(), host) != it->second.server_names.end() && std::find(it->second.ports.begin(), it->second.ports.end(), port) != it->second.ports.end())
                {
                    server_index = it->second.server_index;
                    break;
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
        client.response += "Content-Length: " + std::to_string(file_size) + "\r\n";
        client.response += "Connection: " + conection + "\r\n";
        client.response += "\r\n";
        client.bytes_sent = ((double)client.response.size()  * -1) - 1;
    }
    else
        client.response += body;
    
}
std::string getlocation(client_info &client, server_config &server)
{
    // std::cout << "in getlocation funciton" << std::endl;
    
    std::string uri = client.uri;
    while (uri.size() > 0)
    {
        size_t pos = uri.find_last_of("/");
        if (pos == std::string::npos)
            break;
        if (pos == 0 && uri.size() > 1)
            pos++;
        // std::cout << "location: " << uri << std::endl;
        std::map<std::string, location>::iterator it = server.locations.find(uri);
        if (it != server.locations.end())
        {
            // std::cout << "found location: " << it->first << std::endl;
            return it->first;
        }
        uri = uri.substr(0, pos);
    }

    return "";
}
std::string getcorectserver_path(client_info &client, std::map<int, server_config> &server)
{
    // std::cout << "in getcorect_path funciton" << std::endl;
    int server_index = findMatchingServer(client, server);

    std::string loc = getlocation(client, server[server_index]);
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
    // exit(0);
    // std::transform(headers.begin(), headers.end(), headers.begin(), ::tolower);
    //transform what is before : in evry line to lower case
    // size_t pos = 0;
    // while ((pos = headers.find(":", pos)) != std::string::npos)
    // {
    //     size_t ind = pos;
    //     // std::cout << "ind: " << ind << std::endl;
    //     while(ind > 0 && headers[ind - 1] != '\n'){ 
    //         ind--;
    //         headers[ind] = tolower(headers[ind]);
            
    //     }
    //     while (headers[pos] && headers[pos] != '\r' && headers[pos + 1] != '\n')
    //         pos++;
    // }
    return headers;
}
void handleCgi(client_info &client, std::map<int, server_config> &server, std::string &path)
{
    std::cout << "in cgi funciton" << std::endl;
    int fd;
    std::string body;
    std::string content_type = "";
    char filename[] = "cgi_outputXXXXXX";
    int fdin ;
    char filein[] = "cgi_inputXXXXXX";
    fdin = mkstemp(filein);
    fd = mkstemp(filename);// create a temporary file whit unique name
    if (fd == -1)
    {
        std::cerr << "mkstemp failed" << std::endl;
        return;
    }
    pid_t pid = fork();
    if (pid == -1)
    {
        std::cerr << "fork failed" << std::endl;
        return;
    }
    else if (pid == 0)
    {
        dup2(fd, STDOUT_FILENO);
        close(fd);
        if(client.method.find("POST") != std::string::npos)
        {
            // std::cerr << "in post" << std::endl;
            std::istringstream iss(client.headers["content-length"]);
            size_t content_length;
            iss >> content_length;
            // std::cerr << "client.data: " << client.chunkData << std::endl;
            std::string content = client.chunkData;
            // std::cerr << "content_length: " << content.size() << std::endl;
            write(fdin, content.c_str(), content.size());
            // std::ofstream file("test.txt");
            // file.write(content.c_str(), content.size());
            // file.close();
            
            // exit(0);
        }
        std::vector<std::string> envStrings;
        // else
        // envStrings.push_back("REQUEST_METHOD=GET");
        // std::cerr << client.method << std::endl;
        envStrings.push_back("REQUEST_METHOD=" + client.method);
        // envStrings.push_back("REQUEST_URI=" + client.uri);
        // envStrings.push_back("HTTP_USER_AGENT=" + client.headers["user-agent"]);
        envStrings.push_back("SCRIPT_NAME=" + client.uri);
        envStrings.push_back("PATH_INFO=" + client.path_info);
        envStrings.push_back("QUERY_STRING=" + client.query);
        envStrings.push_back("CONTENT_TYPE=" + client.ContentType);
        envStrings.push_back("CONTENT_LENGTH=" + std::to_string(client.chunkData.size()));
        envStrings.push_back("SERVER_NAME=" + client.headers["host"].substr(0, client.headers["host"].find(":")));
        envStrings.push_back("SERVER_PORT=" + client.headers["host"].substr(client.headers["host"].find(":") + 1));
        envStrings.push_back("SERVER_PROTOCOL=" + client.version);
        envStrings.push_back("GATEWAY_INTERFACE=CGI/1.1");
        envStrings.push_back("REMOTE_ADDR=" + client.headers["host"].substr(0, client.headers["host"].find(":")));
        envStrings.push_back("PATH_TRANSLATED=" + getcorectserver_path(client, server) + client.uri);
        // if(client.headers["cookie"] != "")
            // envStrings.push_back("HTTP_COOKIE=" + client.headers["cookie"]);
        for (std::map<std::string, std::string>::iterator it = client.headers.begin(); it != client.headers.end(); ++it)
        {
            std::string header = it->first;
            std::transform(header.begin(), header.end(), header.begin(), ::toupper);
            envStrings.push_back("HTTP_" + header + "=" + it->second);
        }
        
        // std::cerr << "HTTP_COOKIE: " << client.headers["cookie"] << std::endl;
        envStrings.push_back("REDIRECT_STATUS=200");
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
        if (path.substr(path.find_last_of(".") + 1) == "php")
            cgi_path = (char *)"CGI/php-cgi";
        else
            cgi_path = (char *)"CGI/python-cgi";
        char *args[] = {cgi_path, (char *)path.c_str(), NULL};
        
        alarm(5);
        if(execve(args[0], args, envp) == -1)
            exit(1);
        exit(0); // execve only returns on error
    }
    else
    {
        // parent process
        int status;
        waitpid(pid, &status, 0);
        alarm(0);
        if (WIFEXITED(status))
        {
            int exitstatus = WEXITSTATUS(status);
            if (exitstatus != 0)
            {
                std::cerr << "CGI process exited with status: " << exitstatus << std::endl;
                error_response(client, server[client.index_server], 500); // 500
                close(fd);
                unlink(filename);
                close(fdin);
                unlink(filein);
                return;
            }
        }else if (WIFSIGNALED(status))
        {
            int signal = WTERMSIG(status);
            if(signal == SIGALRM)
                error_response(client, server[client.index_server], 504); // 504
            else
                error_response(client, server[client.index_server], 500); // 500
            close(fd);
            unlink(filename);
            close(fdin);
            unlink(filein);
            return;
        }
        lseek(fd, 0, SEEK_SET);
        char buffer[1024];
        int bytes_read;
        std::string headers = readheadercgi(fd, body);
        if(client.bytes_sent <= 0 && client.bytes_sent != -1 )
        {
            while ((bytes_read = read(fd, buffer, sizeof(buffer))) > 0)
                body+= std::string(buffer, bytes_read);
            // if(headers.size() > 0)
            //     content_type = headers.substr(headers.find("content-type:") + 13, headers.find("\r\n", headers.find("content-type:")) - headers.find("content-type:") - 13);
            // if(headers.find("Set-Cookie:") != std::string::npos)
            // {
            //     std::string cookie = headers.substr(headers.find("Set-Cookie:") + 11, headers.find("\r\n", headers.find("Set-Cookie:")) - headers.find("Set-Cookie:") - 11);
            //     std::cout << "cookie: " << cookie << std::endl;
            // }
            content_type = headers.erase(headers.find("\r\n\r\n"), 4);
            // std::cout << "Content-Type: " << content_type << std::endl;
            // if(content_type.size() > 0)
            //     content_type = content_type.substr(0, content_type.find("\r\n"));
            std::cout << "contxxxxxxxxent_type: " << content_type << std::endl;
            // exit(0);
            //get child process exit status
            close(fd);
            unlink(filename);
            close(fdin);
            unlink(filein);
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
        //try to read if she return 0 file end reached
        if(read(fd, buffer, sizeof(buffer)) == 0)
        {
            client.datafinished = 1;
        }
        alarm(0);
        close(fd);
        unlink(filename);
        close(fdin);
        unlink(filein);
        success(client, body, false, path, content_type);
        return;
    }
    close(fd);
    unlink(filename);
    close(fdin);
    unlink(filein);
    (void)server;
}
void sendbodypart(client_info &client, std::string path)
{
    std::string body;
    if(client.bytes_sent == -1)
        client.bytes_sent = 0;
    // std::cout << "path: " << path << std::endl;
    std::ifstream file(path.c_str());
    char *buffer = new char[READ_BUFFER_SIZE];
    file.seekg(0, std::ios::beg);                 // move to the beginning
    file.seekg(client.bytes_sent, std::ios::beg); // move to byte  from the beginning
    // std::cout << "client.bytes_sent: " << client.bytes_sent << std::endl;
    file.read(buffer, READ_BUFFER_SIZE);
    if (file.eof())
    {
        client.datafinished = 1, std::cout << "file end reached" << std::endl;
    }
    body = std::string(buffer, file.gcount());
    delete[] buffer;

    success(client, body, false);
    file.close();
}
bool handlepathinfo(client_info &client){
    bool is_php = false;
    bool is_cgi = false;
    size_t pos = client.uri.find(".php");
    if (pos == std::string::npos)
        pos = client.uri.find(".py");
    else
        is_php = true;
    if (pos != std::string::npos && (client.uri[pos + is_php + 3] == '\\' || client.uri[pos + is_php + 3] == '?'))
    {
        int add = 3 + is_php;
        std::string path_info = client.uri.substr(pos + add);
        client.path_info = path_info;
        client.uri = client.uri.substr(0, pos + add);
        if(client.path_info.find("?") != std::string::npos)
        {
            client.query = client.path_info.substr(client.path_info.find("?") + 1);
            client.path_info = client.path_info.substr(0, client.path_info.find("?"));
        }
    }
    //chek if the path end with .php or .py
    size_t dot = client.uri.find_last_of(".");
    if (dot != std::string::npos)
    {
        std::string ext = client.uri.substr(dot + 1);
        if (ext == "php" || ext == "py")
            is_cgi = true;
    }
    std::cout << "is_cgi: " << is_cgi << std::endl;
    // exit(0);
    return is_cgi;
    // else
    //     client.path_info = "";
}
void handleGetRequest(client_info &client, std::map<int, server_config> &server)
{
    // exit(0);
    if(client.error_code != 0)
    {
        error_response(client, server[client.index_server], client.error_code); // 500
        return;
    }
    std::cout << "in get funciton" << std::endl;
    client.response.clear();
    std::string content_type = "";
    // std::cout << "client.uri: " << client.uri << std::endl;
    long content_size = -1;
    handlepathinfo(client);
    // std::cout << "path info: " << client.path_info << std::endl;
    // std::cout << "uri: " << client.uri << std::endl;
    // exit(0);
    std::string path = getcorectserver_path(client, server) + client.uri;
    std::ifstream file(path.c_str());
    if (!file.is_open())
    {
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
        return;
    }
    if(path.find_last_of(".") != std::string::npos)
    {
        if(path.substr(path.find_last_of(".") + 1) == "php" || path.substr(path.find_last_of(".") + 1) == "py")
        {
            handleCgi(client, server, path);
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
    std::cout << "in delete funciton" << std::endl;
    std::string path = getcorectserver_path(client, server) + client.uri;
    if (std::remove(path.c_str()) != 0)
    {
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
        client.response += "Content-Length: " + std::to_string(body.size()) + "\r\n";
        client.response += "Connection: keep-alive\r\n";
        client.response += "\r\n";
        client.response += body;
        client.bytes_sent = ((double)client.response.size()  * -1) - 1;
        
        client.poll_status = 1;
        client.datafinished = 1;
    }
    (void)client;
    (void)server;
}

