// LibraryMaster - JavaScript for managing book library
// This script handles navigation, searching, adding books, and displaying book details
// Ensure the DOM is fully loaded before executing the script
if (typeof document === 'undefined') {
    throw new Error('This script must be run in a browser environment.');
}
if (typeof fetch === 'undefined') {
    throw new Error('Fetch API is required for this script to work. Please use a modern browser.');
}
// Ensure the DOM is fully loaded before executing the script
if (typeof document === 'undefined') {
    throw new Error('This script must be run in a browser environment.');
}
document.addEventListener('DOMContentLoaded', function() {
    // DOM Elements
    const navLinks = document.querySelectorAll('.nav-link');
    const sections = document.querySelectorAll('section');
    const searchButton = document.getElementById('search-button');
    const searchType = document.getElementById('search-type');
    const searchQuery = document.getElementById('search-query');
    const resultsContainer = document.getElementById('results-container');
    const noResults = document.getElementById('no-results');
    const addFromSearch = document.getElementById('add-from-search');
    const addBookForm = document.getElementById('add-book-form');
    const addMessage = document.getElementById('add-message');
    const refreshBooksButton = document.getElementById('refresh-books');
    const allBooksContainer = document.getElementById('all-books');
    const bookModal = document.getElementById('book-modal');
    const modalTitle = document.getElementById('modal-title');
    const modalContent = document.getElementById('modal-content');
    const closeModal = document.querySelector('.close');
    
    // Navigation functionality
    navLinks.forEach(link => {
        link.addEventListener('click', function(e) {
            e.preventDefault();
            
            // Update active link
            navLinks.forEach(l => l.classList.remove('active'));
            this.classList.add('active');
            
            // Show corresponding section
            const targetSection = this.getAttribute('data-section');
            sections.forEach(section => {
                section.classList.remove('active');
                if (section.id === targetSection) {
                    section.classList.add('active');
                    
                    // Load all books when navigating to "all" section
                    if (targetSection === 'all') {
                        loadAllBooks();
                    }
                }
            });
        });
    });
    
    // Search functionality
    searchButton.addEventListener('click', function() {
        const type = searchType.value;
        const query = searchQuery.value.trim();
        
        if (query) {
            searchBooks(type, query);
        } else {
            showMessage(addMessage, 'Please enter a search term', 'error');
        }
    });
    
    // Add from search button
    addFromSearch.addEventListener('click', function() {
        // Switch to add book section
        navLinks.forEach(l => l.classList.remove('active'));
        document.querySelector('[data-section="add"]').classList.add('active');
        
        sections.forEach(section => {
            section.classList.remove('active');
            if (section.id === 'add') {
                section.classList.add('active');
                
                // Pre-fill the form if possible
                const query = searchQuery.value.trim();
                const type = searchType.value;
                
                if (type === 'title') {
                    document.getElementById('book-title').value = query;
                } else if (type === 'author') {
                    document.getElementById('book-author').value = query;
                } else if (type === 'year' && !isNaN(parseInt(query))) {
                    document.getElementById('book-year').value = query;
                }
            }
        });
    });
    
    // Add book form submission
    addBookForm.addEventListener('submit', function(e) {
        e.preventDefault();
        
        const title = document.getElementById('book-title').value.trim();
        const author = document.getElementById('book-author').value.trim();
        const yearStr = document.getElementById('book-year').value.trim();
        const quantityStr = document.getElementById('book-quantity').value.trim();
        
        // Form validation
        if (!title || !author || !yearStr || !quantityStr) {
            showMessage(addMessage, 'Please fill in all fields', 'error');
            return;
        }
        
        const year = parseInt(yearStr);
        const quantity = parseInt(quantityStr);
        
        if (isNaN(year) || year < 1000 || year > 9999) {
            showMessage(addMessage, 'Please enter a valid year (1000-9999)', 'error');
            return;
        }
        
        if (isNaN(quantity) || quantity < 1) {
            showMessage(addMessage, 'Please enter a valid quantity (minimum 1)', 'error');
            return;
        }
        
        // Add the book
        addBook(title, author, year, quantity);
    });
    
    // Refresh books button
    refreshBooksButton.addEventListener('click', loadAllBooks);
    
    // Close modal
    closeModal.addEventListener('click', function() {
        bookModal.style.display = 'none';
    });
    
    // Close modal when clicking outside of it
    window.addEventListener('click', function(e) {
        if (e.target === bookModal) {
            bookModal.style.display = 'none';
        }
    });
    
    // Search for books
    function searchBooks(type, query) {
        let url = `/api/search?${type}=${encodeURIComponent(query)}`;
        
        fetch(url)
            .then(response => response.json())
            .then(books => {
                // Clear previous results
                resultsContainer.innerHTML = '';
                
                if (books.length === 0) {
                    // Show "no results" message
                    noResults.classList.remove('hidden');
                } else {
                    // Hide "no results" message
                    noResults.classList.add('hidden');
                    
                    // Create book cards
                    books.forEach(book => {
                        resultsContainer.appendChild(createBookCard(book));
                    });
                }
            })
            .catch(error => {
                console.error('Error searching books:', error);
                resultsContainer.innerHTML = '<p class="error">Error searching books. Please try again.</p>';
            });
    }
    
    // Add a new book
    function addBook(title, author, year, quantity) {
        fetch('/api/books', {
            method: 'POST',
            headers: {
                'Content-Type': 'application/json'
            },
            body: JSON.stringify({
                title: title,
                author: author,
                year: year,
                quantity: quantity
            })
        })
        .then(response => response.json())
        .then(data => {
            if (data.success) {
                // Show success message
                showMessage(addMessage, 'Book added successfully!', 'success');
                
                // Reset form
                addBookForm.reset();
            } else {
                // Show error message
                showMessage(addMessage, data.error || 'Failed to add book', 'error');
            }
        })
        .catch(error => {
            console.error('Error adding book:', error);
            showMessage(addMessage, 'Error adding book. Please try again.', 'error');
        });
    }
    
    // Load all books
    function loadAllBooks() {
        allBooksContainer.innerHTML = '<p class="loading">Loading books...</p>';
        
        fetch('/api/books')
            .then(response => response.json())
            .then(books => {
                allBooksContainer.innerHTML = '';
                
                if (books.length === 0) {
                    allBooksContainer.innerHTML = '<p class="empty-state">No books in the library yet.</p>';
                } else {
                    books.forEach(book => {
                        allBooksContainer.appendChild(createBookCard(book));
                    });
                }
            })
            .catch(error => {
                console.error('Error loading books:', error);
                allBooksContainer.innerHTML = '<p class="error">Error loading books. Please try again.</p>';
            });
    }
    
    // Create a book card
    function createBookCard(book) {
        const card = document.createElement('div');
        card.className = 'book-card';
        card.innerHTML = `
            <div class="book-card-header">
                <h4>${book.title}</h4>
            </div>
            <div class="book-card-body">
                <p><strong>Author:</strong> ${book.author}</p>
                <p><strong>Year:</strong> ${book.year}</p>
                <p class="book-quantity"><strong>Available:</strong> ${book.quantity}</p>
            </div>
        `;
        
        // Add click event to show details
        card.addEventListener('click', function() {
            showBookDetails(book);
        });
        
        return card;
    }
    
    // Show book details in modal
    function showBookDetails(book) {
        modalTitle.textContent = book.title;
        modalContent.innerHTML = `
            <p><strong>Title:</strong> ${book.title}</p>
            <p><strong>Author:</strong> ${book.author}</p>
            <p><strong>Publication Year:</strong> ${book.year}</p>
            <p><strong>Available Quantity:</strong> ${book.quantity}</p>
        `;
        bookModal.style.display = 'block';
    }
    
    // Show message function
    function showMessage(element, message, type) {
        element.textContent = message;
        element.className = `message ${type}`;
        element.classList.remove('hidden');
        
        // Hide message after 5 seconds
        setTimeout(() => {
            element.classList.add('hidden');
        }, 5000);
    }
    
    // Initial load of all books
    loadAllBooks();
});
