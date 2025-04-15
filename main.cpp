#include <iostream>
#include <string>
#include <csignal>
#include "library.h"
#include "server.h"

// Global server pointer for signal handling
Server* serverPtr = nullptr;

// Signal handler for graceful shutdown
void signalHandler(int signal) {
    if (serverPtr) {
        std::cout << "Shutting down server..." << std::endl;
        serverPtr->stop();
    }
    exit(signal);
}

int main(int argc, char* argv[]) {
    // Register signal handlers
    signal(SIGINT, signalHandler);  // Ctrl+C
    signal(SIGTERM, signalHandler); // Termination signal
    
    // Initialize the library with a data file
    std::string dataFile = "library.dat";
    Library library(dataFile);
    
    // Create and start the server
    Server server(library, 5000);
    serverPtr = &server;
    
    std::cout << "Library Management System" << std::endl;
    std::cout << "-------------------------" << std::endl;
    std::cout << "Web interface available at http://localhost:5000" << std::endl;
    
    // Start the server
    if (!server.start()) {
        std::cerr << "Failed to start server" << std::endl;
        return 1;
    }
    
    return 0;
}
