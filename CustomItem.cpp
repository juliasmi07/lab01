#include "CustomItem.h"
#include <sstream>
#include <iomanip>


CustomItem::CustomItem(std::string size) :IceCreamItem(size) {
    if (size == "small") {
         price = 3.00;
    }
    else if(size == "medium") {
        price = 5.00;
    }
    else if (size == "large") {
        price = 6.50;
    }
}

CustomItem::~CustomItem() {}

void CustomItem::addTopping(std::string topping) {
    toppings[topping]++;
    price += 0.40;
}

std::string CustomItem::composeItem() {
    std::ostringstream out;
    out << "Custom Size: " << size << "\n";
    out << "Toppings:\n";
    for (const auto& t : toppings) {
        out << t.first << ": " << t.second << " oz\n";
    }
    out << "Price: $" << std::fixed << std::setprecision(2) << price << "\n";
    return out.str();
}

double CustomItem::getPrice() {
    return price;
}