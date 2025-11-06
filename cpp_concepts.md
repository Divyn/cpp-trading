# C++ Concepts and Names Used in This Project

This document explains all C++ concepts, keywords, types, and names used across the trading project files.

---

## Table of Contents
1. [Standard Library Headers](#standard-library-headers)
2. [Data Types](#data-types)
3. [Keywords](#keywords)
4. [Standard Library Classes and Functions](#standard-library-classes-and-functions)
5. [Language Features](#language-features)
6. [Memory Management](#memory-management)
7. [External Libraries](#external-libraries)

---

## Standard Library Headers

### `<iostream>`
- **Purpose**: Input/Output stream library
- **Used in**: All files
- **Provides**: `std::cout`, `std::cerr`, `std::endl` for console output
- **What it does**: Enables reading from and writing to the standard input/output streams

### `<string>`
- **Purpose**: String manipulation library
- **Used in**: All files
- **Provides**: `std::string` class for string operations
- **What it does**: Provides a string class for handling text data (instead of C-style char arrays)

### `<map>`
- **Purpose**: Associative container library
- **Used in**: `orderbook.cpp`
- **Provides**: `std::map` template class
- **What it does**: Stores key-value pairs in sorted order. Used for mapping prices to quantities in the order book

### `<vector>`
- **Purpose**: Dynamic array container library
- **Used in**: `orderbook.cpp`
- **Provides**: `std::vector` template class
- **What it does**: Provides a dynamic array that can grow/shrink. Used to store collections of orders

### `<regex>`
- **Purpose**: Regular expression library
- **Used in**: `btc_price.cpp`
- **Provides**: `std::regex`, `std::smatch`, `std::regex_search`
- **What it does**: Enables pattern matching in strings. Used to extract price from JSON response

### `<iomanip>`
- **Purpose**: Input/Output manipulators library
- **Used in**: `btc_price.cpp`, `orderbook.cpp`
- **Provides**: `std::fixed`, `std::setprecision`
- **What it does**: Provides formatting functions for I/O streams. Used to format decimal numbers

### `<sstream>`
- **Purpose**: String stream library
- **Used in**: `btc_price.cpp` (included but not actively used)
- **Provides**: `std::stringstream`, `std::ostringstream`, `std::istringstream`
- **What it does**: Allows treating strings as streams for parsing/formatting

### `<curl/curl.h>`
- **Purpose**: libcurl library for HTTP requests
- **Used in**: `btc_price.cpp`
- **Provides**: CURL functions for making HTTP requests
- **What it does**: External C library for making HTTP/HTTPS requests to APIs

---

## Data Types

### Fundamental Types

#### `int`
- **Purpose**: Integer data type
- **Used for**: Order IDs, quantities, counters
- **Size**: Typically 32 bits (4 bytes)
- **Range**: -2,147,483,648 to 2,147,483,647

#### `double`
- **Purpose**: Double-precision floating-point number
- **Used for**: Prices, calculations
- **Size**: Typically 64 bits (8 bytes)
- **Precision**: ~15-17 decimal digits
- **Examples**: `123.45`, `0.0`

#### `bool`
- **Purpose**: Boolean data type
- **Used for**: Flags like `is_buy` (true/false)
- **Size**: Typically 1 byte
- **Values**: `true` or `false`

#### `char`
- **Purpose**: Character data type
- **Used for**: String character access
- **Size**: 1 byte (8 bits)
- **Example**: In pointer casting `(char*)contents`

#### `void`
- **Purpose**: Represents "no type"
- **Used for**: 
  - Function return type when function doesn't return a value
  - Generic pointer type `void*` for callbacks
- **Example**: `WriteCallback(void *contents, ...)`

#### `size_t`
- **Purpose**: Unsigned integer type for sizes
- **Used in**: Callback function signature
- **What it does**: Represents the size of objects in bytes. Used by library functions that return sizes
- **Size**: Typically 64 bits on 64-bit systems

### User-Defined Types

#### `struct Order`
- **Purpose**: Represents a trading order
- **Defined in**: `orderbook.cpp`, `trading_object.cpp`
- **Members**:
  - `int id`: Unique order identifier
  - `double price`: Order price
  - `int quantity`: Number of shares/units
  - `bool is_buy` (orderbook.cpp): `true` for buy, `false` for sell
  - `std::string side` (trading_object.cpp): "BUY" or "SELL" as string
- **What it does**: Groups related data together for a single order

#### `class OrderBook`
- **Purpose**: Represents an order book for trading
- **Defined in**: `orderbook.cpp`
- **What it does**: Maintains buy and sell orders organized by price, calculates best bid/ask, and displays order book data

### Standard Library Types

#### `std::string`
- **Purpose**: String class for text manipulation
- **Used for**: Storing text data, API responses, error messages
- **What it does**: Provides safe, convenient string operations compared to C-style char arrays
- **Features**: Automatic memory management, concatenation, searching

#### `std::map<double, int, std::greater<double>>`
- **Purpose**: Ordered map with custom comparator
- **Used for**: Bid side of order book (highest price first)
- **Template parameters**:
  - `double`: Key type (price)
  - `int`: Value type (quantity)
  - `std::greater<double>`: Comparator (sorts descending)
- **What it does**: Maintains prices sorted in descending order for efficient best bid lookup

#### `std::map<double, int>`
- **Purpose**: Ordered map with default comparator
- **Used for**: Ask side of order book (lowest price first)
- **Template parameters**:
  - `double`: Key type (price)
  - `int`: Value type (quantity)
- **What it does**: Maintains prices sorted in ascending order for efficient best ask lookup

#### `std::vector<Order>`
- **Purpose**: Dynamic array of Order objects
- **Used for**: Storing collections of orders
- **What it does**: Provides a resizable array that can hold multiple Order objects

#### `std::regex`
- **Purpose**: Regular expression object
- **Used for**: Pattern matching in JSON response
- **What it does**: Compiles a regex pattern for searching strings

#### `std::smatch`
- **Purpose**: Match results container
- **Used for**: Storing regex search results
- **What it does**: Holds the matched groups and positions from a regex search

### External Library Types

#### `CURL*`
- **Purpose**: Opaque pointer to libcurl handle
- **Used for**: Managing HTTP request sessions
- **What it does**: Represents a curl session/context for making HTTP requests

#### `CURLcode`
- **Purpose**: Return code type from curl functions
- **Used for**: Error checking
- **Values**: `CURLE_OK` (success) or error codes

#### `curl_slist*`
- **Purpose**: Linked list for HTTP headers
- **Used for**: Setting custom HTTP headers
- **What it does**: Stores key-value pairs for HTTP request headers

---

## Keywords

### Access Modifiers

#### `private`
- **Purpose**: Access specifier for class members
- **Used in**: `OrderBook` class
- **What it does**: Makes members accessible only within the class. Used for `bids` and `asks` maps

#### `public`
- **Purpose**: Access specifier for class members
- **Used in**: `OrderBook` class
- **What it does**: Makes members accessible from outside the class. Used for member functions

### Storage Class Specifiers

#### `static`
- **Purpose**: Internal linkage for functions
- **Used in**: `WriteCallback` function
- **What it does**: Makes the function local to the translation unit (file), not visible to other files

### Type Qualifiers

#### `const`
- **Purpose**: Immutability qualifier
- **Used for**:
  - Function parameters: `const Order& order` - parameter cannot be modified
  - Member functions: `void printTopOfBook() const` - function doesn't modify object state
  - Variables: `const char *data` - pointer to constant data
- **What it does**: Prevents modification, enables better optimization, expresses intent

### Control Flow

#### `if/else`
- **Purpose**: Conditional execution
- **Used for**: Decision making based on conditions
- **Example**: `if (btcPrice > 0.0) { ... }`

#### `return`
- **Purpose**: Exit function and return value
- **Used for**: Returning values from functions or exiting early
- **Example**: `return price;`, `return 0;`

### Exception Handling

#### `try`
- **Purpose**: Exception handling block
- **Used in**: `btc_price.cpp` for price parsing
- **What it does**: Marks a block where exceptions may occur

#### `catch`
- **Purpose**: Exception handler
- **Used with**: `try` block
- **Syntax**: `catch (const std::exception& e)`
- **What it does**: Catches and handles exceptions thrown in the `try` block

---

## Standard Library Classes and Functions

### I/O Stream Functions

#### `std::cout`
- **Purpose**: Standard output stream
- **What it does**: Writes to console/terminal
- **Usage**: `std::cout << "Hello" << std::endl;`

#### `std::cerr`
- **Purpose**: Standard error stream
- **What it does**: Writes error messages to console
- **Usage**: `std::cerr << "Error: " << message;`

#### `std::endl`
- **Purpose**: End line manipulator
- **What it does**: Inserts newline and flushes the output buffer
- **Usage**: `std::cout << "text" << std::endl;`

#### `std::fixed`
- **Purpose**: Floating-point format manipulator
- **What it does**: Sets fixed-point notation (not scientific)
- **Usage**: `std::cout << std::fixed << price;`

#### `std::setprecision(int)`
- **Purpose**: Precision manipulator
- **What it does**: Sets number of decimal places to display
- **Usage**: `std::cout << std::setprecision(2) << price;`

### String Functions

#### `std::string::append()`
- **Purpose**: Append characters to string
- **What it does**: Adds characters to the end of a string
- **Usage**: `((std::string*)userp)->append((char*)contents, size * nmemb);`

#### `std::string::str()`
- **Purpose**: Get string from match result
- **What it does**: Extracts the matched substring from regex result
- **Usage**: `match[1].str()`

#### `std::stod()`
- **Purpose**: String to double conversion
- **What it does**: Converts a string to a double-precision floating-point number
- **Throws**: `std::invalid_argument` or `std::out_of_range` on failure
- **Usage**: `price = std::stod(match[1].str());`

### Regular Expression Functions

#### `std::regex_search()`
- **Purpose**: Search for regex pattern in string
- **Parameters**:
  - String to search
  - Match results container
  - Regex pattern
- **Returns**: `true` if pattern found, `false` otherwise
- **Usage**: `std::regex_search(readBuffer, match, priceRegex);`

### Container Functions

#### `std::map::begin()`
- **Purpose**: Get iterator to first element
- **What it does**: Returns iterator pointing to the first (lowest/highest) element
- **Usage**: `bids.begin()->first` (gets first key)

#### `std::map::empty()`
- **Purpose**: Check if container is empty
- **Returns**: `true` if no elements, `false` otherwise
- **Usage**: `if (bids.empty()) return 0.0;`

#### `std::map::erase()`
- **Purpose**: Remove element from map
- **What it does**: Deletes the element with the specified key
- **Usage**: `bids.erase(price);`

#### `std::map::operator[]`
- **Purpose**: Access or insert element
- **What it does**: Returns reference to value at key, creates entry if doesn't exist
- **Usage**: `bids[order.price] += order.quantity;`

#### `std::vector` Initialization
- **Purpose**: Initialize vector with values
- **Syntax**: `std::vector<Order> orders = { Order(...), Order(...) };`
- **What it does**: Creates vector and populates with initial values

### Iterators and Range-Based For Loops

#### Range-Based For Loop
- **Syntax**: `for (const auto& [price, quantity] : bids) { ... }`
- **Purpose**: Iterate over container elements
- **What it does**: 
  - `const auto&`: Automatic type deduction, const reference
  - `[price, quantity]`: Structured binding (C++17) - unpacks map key-value pairs
  - Iterates through all elements in the container

---

## Language Features

### Functions

#### Function Definition
- **Syntax**: `returnType functionName(parameters) { body }`
- **Example**: `double getLatestBTCPrice() { ... }`
- **What it does**: Defines a reusable block of code

#### Function Parameters
- **By value**: `void func(int x)` - copies the value
- **By reference**: `void func(const Order& order)` - no copy, direct access
- **By pointer**: `void func(void *contents)` - passes memory address

#### Return Types
- **With value**: `double getLatestBTCPrice()` - returns a value
- **Void**: `void addOrder(...)` - doesn't return anything

### Classes and Structs

#### `struct`
- **Purpose**: Aggregate data type
- **What it does**: Groups related variables together
- **Default access**: All members are public
- **Example**: `struct Order { ... };`

#### `class`
- **Purpose**: User-defined type with data and functions
- **What it does**: Encapsulates data and behavior
- **Default access**: All members are private
- **Example**: `class OrderBook { ... };`

#### Member Functions
- **Purpose**: Functions that belong to a class
- **Syntax**: `returnType ClassName::functionName() { ... }`
- **What it does**: Operates on class data members
- **Example**: `void OrderBook::addOrder(const Order& order)`

#### Constructor
- **Purpose**: Initialize object when created
- **Syntax**: `Order(int i, double p, int q, bool buy) : id(i), price(p), quantity(q), is_buy(buy) {}`
- **What it does**: 
  - Initialization list (`: id(i), ...`) - more efficient than assignment
  - Sets initial values for all members

#### Const Member Functions
- **Syntax**: `void printTopOfBook() const`
- **Purpose**: Function that doesn't modify object state
- **What it does**: Can be called on const objects, promises not to change member variables

### Pointers and References

#### Pointers (`*`)
- **Purpose**: Store memory address
- **Declaration**: `Order* orderPtr`
- **Usage**: 
  - `new Order{...}` - allocate memory
  - `orderPtr->id` - access member via pointer
  - `delete orderPtr` - free memory
- **What it does**: Points to location in memory where object exists

#### References (`&`)
- **Purpose**: Alias for existing variable
- **Usage**: `const Order& order` - reference parameter
- **What it does**: No copy is made, works directly with original object
- **Benefits**: More efficient than copying, especially for large objects

#### Pointer Casting
- **Syntax**: `(char*)contents` - C-style cast
- **Purpose**: Convert pointer type
- **What it does**: Treats `void*` as `char*` for string operations
- **Note**: Can be unsafe, but necessary for callback functions

### Initialization

#### Initialization Lists
- **Syntax**: `Order(...) : id(i), price(p), quantity(q), is_buy(buy) {}`
- **Purpose**: Initialize member variables directly
- **What it does**: More efficient than assignment in constructor body
- **Benefits**: Direct initialization, required for const/reference members

#### Uniform Initialization
- **Syntax**: `Order{101, 123.45, 10, "BUY"}`
- **Purpose**: Initialize object with values
- **What it does**: C++11 feature for cleaner initialization
- **Alternative**: `Order(101, 123.45, 10, "BUY")`

### Template Classes

#### `std::map<Key, Value, Comparator>`
- **Purpose**: Generic associative container
- **Template parameters**:
  - `Key`: Type of keys (e.g., `double`)
  - `Value`: Type of values (e.g., `int`)
  - `Comparator`: Optional, how to sort keys (e.g., `std::greater<double>`)
- **What it does**: Stores sorted key-value pairs, allows efficient lookup

#### `std::vector<Type>`
- **Purpose**: Generic dynamic array
- **Template parameter**: `Type` - element type (e.g., `Order`)
- **What it does**: Resizable array that holds elements of specified type

### Structured Bindings (C++17)

#### Syntax
- `for (const auto& [price, quantity] : bids)`
- **Purpose**: Unpack structured data
- **What it does**: Automatically extracts key-value pairs from map iteration
- **Alternative**: `for (const auto& pair : bids) { pair.first, pair.second }`

### Auto Type Deduction

#### `auto`
- **Purpose**: Let compiler deduce type
- **Usage**: `const auto& [price, quantity] : bids`
- **What it does**: Compiler figures out the type automatically
- **Benefits**: Shorter code, less repetition, works with complex types

---

## Memory Management

### Stack Allocation
- **Purpose**: Automatic memory management
- **Usage**: `OrderBook book;` or `std::vector<Order> orders = {...};`
- **What it does**: Memory allocated on stack, automatically freed when variable goes out of scope
- **Benefits**: Fast, automatic cleanup, no memory leaks

### Dynamic Allocation

#### `new`
- **Purpose**: Allocate memory on heap
- **Usage**: `Order* orderPtr = new Order{101, 123.45, 10, "BUY"};`
- **What it does**: Allocates memory on heap, returns pointer
- **Returns**: Pointer to allocated memory

#### `delete`
- **Purpose**: Free heap memory
- **Usage**: `delete orderPtr;`
- **What it does**: Frees memory allocated with `new`
- **Important**: Must match each `new` with `delete` to prevent memory leaks

### RAII (Resource Acquisition Is Initialization)
- **Concept**: Objects manage their own resources
- **Example**: `std::string` automatically manages its memory
- **What it does**: Resource cleanup happens automatically when object is destroyed
- **Benefits**: Prevents memory leaks, exception-safe

---

## External Libraries

### libcurl (CURL)

#### `curl_easy_init()`
- **Purpose**: Initialize curl session
- **Returns**: `CURL*` handle or NULL on failure
- **What it does**: Creates a new curl session for making HTTP requests

#### `curl_easy_setopt()`
- **Purpose**: Set curl options
- **Parameters**: Handle, option constant, value
- **What it does**: Configures curl behavior (URL, headers, data, callbacks, etc.)
- **Examples**:
  - `CURLOPT_URL`: Set request URL
  - `CURLOPT_POSTFIELDS`: Set POST data
  - `CURLOPT_WRITEFUNCTION`: Set callback for response data
  - `CURLOPT_WRITEDATA`: Set user data pointer for callback

#### `curl_easy_perform()`
- **Purpose**: Execute HTTP request
- **Returns**: `CURLcode` (CURLE_OK on success)
- **What it does**: Performs the configured HTTP request and calls callbacks

#### `curl_easy_strerror()`
- **Purpose**: Get error message string
- **Returns**: Human-readable error message
- **What it does**: Converts error code to descriptive string

#### `curl_easy_cleanup()`
- **Purpose**: Clean up curl session
- **What it does**: Frees resources allocated by curl_easy_init()

#### `curl_slist_append()`
- **Purpose**: Add header to linked list
- **Returns**: Updated list pointer
- **What it does**: Appends HTTP header to the list

#### `curl_slist_free_all()`
- **Purpose**: Free header list
- **What it does**: Releases all memory used by the header list

### Callback Functions

#### Function Pointer as Callback
- **Purpose**: Library calls user-defined function
- **Example**: `WriteCallback` function passed to curl
- **What it does**: Allows library to call custom code (e.g., when data arrives)
- **Signature**: `size_t WriteCallback(void *contents, size_t size, size_t nmemb, void *userp)`
  - `contents`: Pointer to data received
  - `size`: Size of each element
  - `nmemb`: Number of elements
  - `userp`: User-provided pointer (e.g., string buffer)
  - **Returns**: Number of bytes processed

---

## Summary by File

### `btc_price.cpp`
**Concepts Used**:
- HTTP requests (libcurl)
- Callback functions
- String manipulation
- Regular expressions
- Exception handling
- I/O formatting
- Pointer casting

### `orderbook.cpp`
**Concepts Used**:
- Classes and objects
- Structs
- Constructors with initialization lists
- Const member functions
- STL containers (map, vector)
- Custom comparators
- Range-based for loops
- Structured bindings
- Access modifiers (private/public)

### `trading_object.cpp`
**Concepts Used**:
- Structs
- Dynamic memory allocation (new/delete)
- Pointers
- Member access via `->`

---

## Best Practices Demonstrated

1. **RAII**: Using stack-allocated objects and STL containers for automatic memory management
2. **Const Correctness**: Using `const` for parameters and member functions that don't modify state
3. **References**: Using `const&` for large object parameters to avoid copying
4. **Initialization Lists**: Efficient member initialization in constructors
5. **Exception Handling**: Using try-catch for error handling in string conversion
6. **Type Safety**: Using `bool` instead of strings for binary flags (is_buy)

---

## Common Patterns

### Iterator Pattern
- `for (const auto& [key, value] : container)` - iterate over containers

### RAII Pattern
- Objects manage their own resources (e.g., `std::string`, `std::vector`)

### Callback Pattern
- Function pointer passed to library for custom handling (curl callback)

### Encapsulation
- Private data members, public interface (`OrderBook` class)

