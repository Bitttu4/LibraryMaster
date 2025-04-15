#include "library.h"
#include <fstream>
#include <algorithm>
#include <cctype>
#include <sstream>

// Constructor
Library::Library(const std::string& dataFile) : dataFile(dataFile) {
    loadBooks();
}

// Load books from file
bool Library::loadBooks() {
    std::ifstream file(dataFile);
    if (!file.is_open()) {
        return false;
    }
    
    books.clear();
    std::string line;
    
    while (std::getline(file, line)) {
        if (!line.empty()) {
            books.push_back(Book::fromString(line));
        }
    }
    
    file.close();
    return true;
}

// Save books to file
bool Library::saveBooks() const {
    std::ofstream file(dataFile);
    if (!file.is_open()) {
        return false;
    }
    
    for (const auto& book : books) {
        file << book.toString() << std::endl;
    }
    
    file.close();
    return true;
}

// Add a new book
bool Library::addBook(const Book& book) {
    books.push_back(book);
    return saveBooks();
}

// Get all books
std::vector<Book> Library::getAllBooks() const {
    return books;
}

// Helper function to convert string to lowercase for case-insensitive comparison
std::string toLower(const std::string& str) {
    std::string result = str;
    std::transform(result.begin(), result.end(), result.begin(),
                   [](unsigned char c) { return std::tolower(c); });
    return result;
}

// Search books by title (partial match, case-insensitive)
std::vector<Book> Library::searchByTitle(const std::string& title) const {
    std::vector<Book> result;
    std::string searchTerm = toLower(title);
    
    for (const auto& book : books) {
        if (toLower(book.getTitle()).find(searchTerm) != std::string::npos) {
            result.push_back(book);
        }
    }
    
    return result;
}

// Search books by author (partial match, case-insensitive)
std::vector<Book> Library::searchByAuthor(const std::string& author) const {
    std::vector<Book> result;
    std::string searchTerm = toLower(author);
    
    for (const auto& book : books) {
        if (toLower(book.getAuthor()).find(searchTerm) != std::string::npos) {
            result.push_back(book);
        }
    }
    
    return result;
}

// Search books by year
std::vector<Book> Library::searchByYear(int year) const {
    std::vector<Book> result;
    
    for (const auto& book : books) {
        if (book.getYear() == year) {
            result.push_back(book);
        }
    }
    
    return result;
}

// Convert books vector to JSON array
std::string Library::booksToJson(const std::vector<Book>& books) const {
    std::stringstream ss;
    ss << "[";
    
    for (size_t i = 0; i < books.size(); ++i) {
        ss << books[i].toJson();
        if (i < books.size() - 1) {
            ss << ",";
        }
    }
    
    ss << "]";
    return ss.str();
}
