#include "PreMadeItem.h"
#include <sstream>
#include <iomanip>

PreMadeItem::PreMadeItem(std::string name, std::string size)
    : IceCreamItem(size), name(name) {
    if (size == "small") price = 4.00;
    else if (size == "medium") price = 6.00;
    else if (size == "large") price = 7.50;
}

PreMadeItem::~PreMadeItem() {}

double PreMadeItem::getPrice() {
    return price;
}

std::string PreMadeItem::composeItem() {
    std::ostringstream out;
    out << "Pre-made Size: " << size << "\n";
    out << "Pre-made Item: " << name << "\n";
    out << "Price: $" << std::fixed << std::setprecision(2) << price << "\n";
    return out.str();
}
