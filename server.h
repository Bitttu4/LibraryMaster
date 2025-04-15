#ifndef SERVER_H
#define SERVER_H

#include "library.h"
#include <string>
#include <functional>
#include <map>
#include <vector>

// Simple HTTP response structure
struct HttpResponse {
    int status;
    std::string contentType;
    std::string body;
};

// HTTP method enum
enum class HttpMethod {
    GET,
    POST,
    PUT,
    DELETE
};

// Request handler function type
using RequestHandler = std::function<HttpResponse(const std::map<std::string, std::string>&, const std::string&)>;

// Route structure
struct Route {
    HttpMethod method;
    std::string path;
    RequestHandler handler;
};

class Server {
private:
    Library& library;
    int port;
    std::vector<Route> routes;
    bool running;
    int serverSocket;

    // Internal methods
    void setupRoutes();
    bool setupSocket();
    void handleClient(int clientSocket);
    std::pair<std::string, std::map<std::string, std::string>> parseRequest(const std::string& request);
    HttpResponse routeRequest(HttpMethod method, const std::string& path, 
                             const std::map<std::string, std::string>& params,
                             const std::string& body);
    std::string methodToString(HttpMethod method);
    HttpMethod stringToMethod(const std::string& method);
    std::string readFile(const std::string& path);
    
    // Request handlers
    HttpResponse handleGetAllBooks(const std::map<std::string, std::string>& params, const std::string& body);
    HttpResponse handleSearchBooks(const std::map<std::string, std::string>& params, const std::string& body);
    HttpResponse handleAddBook(const std::map<std::string, std::string>& params, const std::string& body);

public:
    // Constructor
    Server(Library& library, int port);
    
    // Destructor
    ~Server();
    
    // Start the server
    bool start();
    
    // Stop the server
    void stop();
};

#endif // SERVER_H
