#ifndef BOOK_H
#define BOOK_H

#include <string>

class Book {
private:
    std::string title;
    std::string author;
    int year;
    int quantity;

public:
    // Constructor
    Book(const std::string& title, const std::string& author, int year, int quantity);
    
    // Default constructor (needed for vector operations)
    Book();
    
    // Getters
    std::string getTitle() const;
    std::string getAuthor() const;
    int getYear() const;
    int getQuantity() const;
    
    // Setters
    void setTitle(const std::string& title);
    void setAuthor(const std::string& author);
    void setYear(int year);
    void setQuantity(int quantity);
    
    // Convert book to string representation for file storage
    std::string toString() const;
    
    // Parse book from string representation
    static Book fromString(const std::string& str);
    
    // Convert book to JSON format for web interface
    std::string toJson() const;
};

#endif // BOOK_H
