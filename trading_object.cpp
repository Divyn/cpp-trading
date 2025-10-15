#include <iostream>
#include <string>

struct Order {
    int id;
    double price;
    int quantity;
    std::string side; // "BUY" or "SELL"
};

int main() {
    Order* orderPtr = new Order{101, 123.45, 10, "BUY"};

    std::cout << "Order ID: " << orderPtr->id << "\n";
    std::cout << "Price: " << orderPtr->price << "\n";
    std::cout << "Side: " << orderPtr->side << "\n";

    // Modify via pointer
    orderPtr->price = 123.50;
    std::cout << "Updated Price: " << orderPtr->price << "\n";

    delete orderPtr; // Clean up memory
}
