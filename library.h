#ifndef LIBRARY_H
#define LIBRARY_H

#include "book.h"
#include <vector>
#include <string>

class Library {
private:
    std::vector<Book> books;
    std::string dataFile;

public:
    // Constructor
    Library(const std::string& dataFile);
    
    // Load books from file
    bool loadBooks();
    
    // Save books to file
    bool saveBooks() const;
    
    // Add a new book
    bool addBook(const Book& book);
    
    // Get all books
    std::vector<Book> getAllBooks() const;
    
    // Search books by title (partial match)
    std::vector<Book> searchByTitle(const std::string& title) const;
    
    // Search books by author (partial match)
    std::vector<Book> searchByAuthor(const std::string& author) const;
    
    // Search books by year
    std::vector<Book> searchByYear(int year) const;
    
    // Convert all books to JSON array
    std::string booksToJson(const std::vector<Book>& books) const;
};

#endif // LIBRARY_H
