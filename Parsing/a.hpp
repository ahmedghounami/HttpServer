struct FormInfo {
  bool isChunked;
  bool bodyReached;
  bool bodyTaken;
  // std::string body; //re edit
  int contentLength;
  std::string filename;//re edit
  std::string contentType; // re edidt
};

struct client_info {
  std::string chunk;
  std::string boundary;
  std::string method, uri, version;
  FormInfo file;
  std::map<std::string, std::string> headers;
  std::multimap<std::string, std::string> multiheaders;
  std::map<std::string, std::string> dataInfo;

  // std::string header;
  // std::string boundary_end;
};

class server {
private:
  int start_connection;
  std::map<int, client_info> clients;
  std::vector<pollfd> clients_fds;
  std::vector<pollfd> listners;

public:
  server();
  void listen_for_connections();
  ~server();
};

void accept_connection(int start_connection, std::vector<pollfd> &clients_fds,
                       std::map<int, client_info> &clients);

