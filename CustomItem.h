#ifndef CUSTOMITEM_H
#define CUSTOMITEM_H

// learned need endif but then also indef at the top
#include "IceCreamItem.h"
#include <string>
#include <map>


class CustomItem : public IceCreamItem {

    public:

        //Constructor
        CustomItem(std::string size);

        //Destructor
        virtual ~CustomItem();

        //custom ice cream order add ons
        void addTopping(std::string topping);

        virtual std::string composeItem();
        virtual double getPrice();
        
    private:
        std::map<std::string,int> toppings;
};

#endif