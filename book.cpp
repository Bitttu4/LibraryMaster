#include "book.h"
#include <sstream>
#include <iomanip>

// Constructor
Book::Book(const std::string& title, const std::string& author, int year, int quantity)
    : title(title), author(author), year(year), quantity(quantity) {}

// Default constructor
Book::Book() : title(""), author(""), year(0), quantity(0) {}

// Getters
std::string Book::getTitle() const {
    return title;
}

std::string Book::getAuthor() const {
    return author;
}

int Book::getYear() const {
    return year;
}

int Book::getQuantity() const {
    return quantity;
}

// Setters
void Book::setTitle(const std::string& title) {
    this->title = title;
}

void Book::setAuthor(const std::string& author) {
    this->author = author;
}

void Book::setYear(int year) {
    this->year = year;
}

void Book::setQuantity(int quantity) {
    this->quantity = quantity;
}

// Convert book to string representation for file storage
// Format: title|author|year|quantity
std::string Book::toString() const {
    std::stringstream ss;
    ss << title << "|" << author << "|" << year << "|" << quantity;
    return ss.str();
}

// Parse book from string representation
Book Book::fromString(const std::string& str) {
    std::stringstream ss(str);
    std::string title, author, yearStr, quantityStr;
    
    // Parse the fields separated by |
    std::getline(ss, title, '|');
    std::getline(ss, author, '|');
    std::getline(ss, yearStr, '|');
    std::getline(ss, quantityStr, '|');
    
    int year = 0, quantity = 0;
    try {
        year = std::stoi(yearStr);
        quantity = std::stoi(quantityStr);
    } catch (const std::exception& e) {
        // Handle parsing errors
    }
    
    return Book(title, author, year, quantity);
}

// Convert book to JSON format for web interface
std::string Book::toJson() const {
    std::stringstream ss;
    ss << "{" 
       << "\"title\":\"" << title << "\","
       << "\"author\":\"" << author << "\","
       << "\"year\":" << year << ","
       << "\"quantity\":" << quantity 
       << "}";
    return ss.str();
}
