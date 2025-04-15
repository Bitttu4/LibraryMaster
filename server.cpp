#include "server.h"
#include <iostream>
#include <sstream>
#include <fstream>
#include <cstring>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <algorithm>
#include <thread>
#include <vector>

// Constructor
Server::Server(Library& library, int port) : library(library), port(port), running(false), serverSocket(-1) {
    setupRoutes();
}

// Destructor
Server::~Server() {
    stop();
}

// Setup the routes for the server
void Server::setupRoutes() {
    // API routes
    routes.push_back({HttpMethod::GET, "/api/books", 
                    std::bind(&Server::handleGetAllBooks, this, std::placeholders::_1, std::placeholders::_2)});
    routes.push_back({HttpMethod::GET, "/api/search", 
                    std::bind(&Server::handleSearchBooks, this, std::placeholders::_1, std::placeholders::_2)});
    routes.push_back({HttpMethod::POST, "/api/books", 
                    std::bind(&Server::handleAddBook, this, std::placeholders::_1, std::placeholders::_2)});
}

// Setup the server socket
bool Server::setupSocket() {
    // Create socket
    serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket < 0) {
        std::cerr << "Error creating socket" << std::endl;
        return false;
    }
    
    // Set socket options to reuse address
    int opt = 1;
    if (setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        std::cerr << "Error setting socket options" << std::endl;
        close(serverSocket);
        return false;
    }
    
    // Bind socket to port
    struct sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port);
    
    if (bind(serverSocket, (struct sockaddr*)&address, sizeof(address)) < 0) {
        std::cerr << "Error binding socket to port " << port << std::endl;
        close(serverSocket);
        return false;
    }
    
    // Listen for connections
    if (listen(serverSocket, 5) < 0) {
        std::cerr << "Error listening on socket" << std::endl;
        close(serverSocket);
        return false;
    }
    
    return true;
}

// Start the server
bool Server::start() {
    if (running) {
        std::cerr << "Server already running" << std::endl;
        return false;
    }
    
    if (!setupSocket()) {
        return false;
    }
    
    running = true;
    std::cout << "Server running on port " << port << std::endl;
    
    // Accept connections in a loop
    while (running) {
        struct sockaddr_in clientAddress;
        socklen_t clientAddressLength = sizeof(clientAddress);
        int clientSocket = accept(serverSocket, (struct sockaddr*)&clientAddress, &clientAddressLength);
        
        if (clientSocket < 0) {
            if (running) {
                std::cerr << "Error accepting connection" << std::endl;
            }
            continue;
        }
        
        // Handle client in a new thread
        std::thread clientThread([this, clientSocket]() {
            handleClient(clientSocket);
            close(clientSocket);
        });
        clientThread.detach();
    }
    
    return true;
}

// Stop the server
void Server::stop() {
    if (running) {
        running = false;
        if (serverSocket >= 0) {
            close(serverSocket);
            serverSocket = -1;
        }
        std::cout << "Server stopped" << std::endl;
    }
}

// Handle client connection
void Server::handleClient(int clientSocket) {
    char buffer[4096] = {0};
    ssize_t bytesRead = read(clientSocket, buffer, sizeof(buffer) - 1);
    
    if (bytesRead <= 0) {
        return;
    }
    
    // Parse the HTTP request
    std::string request(buffer);
    auto [requestLine, params] = parseRequest(request);
    
    // Parse the method and path from the request line
    std::string method, path, version;
    std::istringstream iss(requestLine);
    iss >> method >> path >> version;
    
    // Find the request body (if any)
    std::string body;
    size_t bodyPos = request.find("\r\n\r\n");
    if (bodyPos != std::string::npos) {
        body = request.substr(bodyPos + 4);
    }
    
    // Route the request and get response
    HttpResponse response = routeRequest(stringToMethod(method), path, params, body);
    
    // Construct the HTTP response
    std::stringstream responseStr;
    responseStr << "HTTP/1.1 " << response.status << " " 
                << (response.status == 200 ? "OK" : (response.status == 404 ? "Not Found" : "Internal Server Error"))
                << "\r\n";
    responseStr << "Content-Type: " << response.contentType << "\r\n";
    responseStr << "Content-Length: " << response.body.size() << "\r\n";
    responseStr << "Connection: close\r\n";
    responseStr << "\r\n";
    responseStr << response.body;
    
    // Send the response
    write(clientSocket, responseStr.str().c_str(), responseStr.str().size());
}

// Parse the HTTP request
std::pair<std::string, std::map<std::string, std::string>> Server::parseRequest(const std::string& request) {
    std::istringstream iss(request);
    std::string requestLine;
    std::getline(iss, requestLine);
    
    // Trim carriage return
    if (!requestLine.empty() && requestLine.back() == '\r') {
        requestLine.pop_back();
    }
    
    // Parse the query parameters
    std::map<std::string, std::string> params;
    std::string method, path, version;
    std::istringstream requestIss(requestLine);
    requestIss >> method >> path >> version;
    
    size_t questionMarkPos = path.find('?');
    if (questionMarkPos != std::string::npos) {
        std::string queryString = path.substr(questionMarkPos + 1);
        path = path.substr(0, questionMarkPos);
        
        std::istringstream queryIss(queryString);
        std::string param;
        while (std::getline(queryIss, param, '&')) {
            size_t equalPos = param.find('=');
            if (equalPos != std::string::npos) {
                std::string key = param.substr(0, equalPos);
                std::string value = param.substr(equalPos + 1);
                params[key] = value;
            }
        }
    }
    
    return {requestLine, params};
}

// Route the request to the appropriate handler
HttpResponse Server::routeRequest(HttpMethod method, const std::string& path, 
                                 const std::map<std::string, std::string>& params,
                                 const std::string& body) {
    // Find the matching route
    for (const auto& route : routes) {
        if (route.method == method && route.path == path) {
            return route.handler(params, body);
        }
    }
    
    // Special case for static files
    if (method == HttpMethod::GET) {
        // Serve static files directly
        std::string content;
        std::string contentType = "text/plain";
        
        if (path == "/") {
            content = readFile("/");
            contentType = "text/html";
        } else if (path == "/styles.css") {
            content = readFile("/styles.css");
            contentType = "text/css";
        } else if (path == "/script.js") {
            content = readFile("/script.js");
            contentType = "application/javascript";
        } else if (path.find("/www/") == 0) {
            content = readFile(path);
            
            if (path.find(".html") != std::string::npos) {
                contentType = "text/html";
            } else if (path.find(".css") != std::string::npos) {
                contentType = "text/css";
            } else if (path.find(".js") != std::string::npos) {
                contentType = "application/javascript";
            } else if (path.find(".json") != std::string::npos) {
                contentType = "application/json";
            } else if (path.find(".svg") != std::string::npos) {
                contentType = "image/svg+xml";
            }
        }
        
        if (!content.empty()) {
            return {200, contentType, content};
        }
    }
    
    // No matching route found
    return {404, "text/plain", "Not Found"};
}

// Convert HTTP method enum to string
std::string Server::methodToString(HttpMethod method) {
    switch (method) {
        case HttpMethod::GET: return "GET";
        case HttpMethod::POST: return "POST";
        case HttpMethod::PUT: return "PUT";
        case HttpMethod::DELETE: return "DELETE";
        default: return "UNKNOWN";
    }
}

// Convert string to HTTP method enum
HttpMethod Server::stringToMethod(const std::string& method) {
    if (method == "GET") return HttpMethod::GET;
    if (method == "POST") return HttpMethod::POST;
    if (method == "PUT") return HttpMethod::PUT;
    if (method == "DELETE") return HttpMethod::DELETE;
    return HttpMethod::GET; // Default to GET
}

// Read file content
std::string Server::readFile(const std::string& path) {
    // Determine the file path
    std::string filePath = "www" + path;
    if (path == "/") {
        filePath = "www/index.html";
    }
    
    // Read the file
    std::ifstream file(filePath, std::ios::binary);
    if (!file.is_open()) {
        return "";
    }
    
    return std::string((std::istreambuf_iterator<char>(file)),
                     std::istreambuf_iterator<char>());
}

// Handler for GET /api/books
HttpResponse Server::handleGetAllBooks(const std::map<std::string, std::string>& params, const std::string& body) {
    std::vector<Book> books = library.getAllBooks();
    std::string json = library.booksToJson(books);
    return {200, "application/json", json};
}

// Handler for GET /api/search
HttpResponse Server::handleSearchBooks(const std::map<std::string, std::string>& params, const std::string& body) {
    std::vector<Book> results;
    
    // Check which search parameter is provided
    auto titleIt = params.find("title");
    auto authorIt = params.find("author");
    auto yearIt = params.find("year");
    
    if (titleIt != params.end()) {
        results = library.searchByTitle(titleIt->second);
    } else if (authorIt != params.end()) {
        results = library.searchByAuthor(authorIt->second);
    } else if (yearIt != params.end()) {
        try {
            int year = std::stoi(yearIt->second);
            results = library.searchByYear(year);
        } catch (const std::exception& e) {
            return {400, "application/json", "{\"error\":\"Invalid year format\"}"};
        }
    } else {
        return {400, "application/json", "{\"error\":\"No search parameter provided\"}"};
    }
    
    std::string json = library.booksToJson(results);
    return {200, "application/json", json};
}

// Handler for POST /api/books
HttpResponse Server::handleAddBook(const std::map<std::string, std::string>& params, const std::string& body) {
    // Simple parsing of JSON body
    // In a real application, you would use a proper JSON parser
    std::string title, author;
    int year = 0, quantity = 0;
    
    size_t titlePos = body.find("\"title\":");
    size_t authorPos = body.find("\"author\":");
    size_t yearPos = body.find("\"year\":");
    size_t quantityPos = body.find("\"quantity\":");
    
    if (titlePos != std::string::npos) {
        size_t start = body.find("\"", titlePos + 8) + 1;
        size_t end = body.find("\"", start);
        title = body.substr(start, end - start);
    }
    
    if (authorPos != std::string::npos) {
        size_t start = body.find("\"", authorPos + 9) + 1;
        size_t end = body.find("\"", start);
        author = body.substr(start, end - start);
    }
    
    if (yearPos != std::string::npos) {
        size_t start = yearPos + 7;
        size_t end = body.find(",", start);
        if (end == std::string::npos) {
            end = body.find("}", start);
        }
        try {
            year = std::stoi(body.substr(start, end - start));
        } catch (const std::exception& e) {
            // Handle parsing error
        }
    }
    
    if (quantityPos != std::string::npos) {
        size_t start = quantityPos + 11;
        size_t end = body.find(",", start);
        if (end == std::string::npos) {
            end = body.find("}", start);
        }
        try {
            quantity = std::stoi(body.substr(start, end - start));
        } catch (const std::exception& e) {
            // Handle parsing error
        }
    }
    
    // Validate input
    if (title.empty() || author.empty() || year <= 0 || quantity <= 0) {
        return {400, "application/json", "{\"error\":\"Invalid book data\"}"};
    }
    
    // Add the book
    Book book(title, author, year, quantity);
    if (library.addBook(book)) {
        return {200, "application/json", "{\"success\":true}"};
    } else {
        return {500, "application/json", "{\"error\":\"Failed to add book\"}"};
    }
}


