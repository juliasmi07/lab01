// ============================================================================
// CS32 P01 Part 2: Sequence implementation
//
// This file is #included at the bottom of Sequence.h rather than compiled on
// its own (templates must be visible wherever they are used). See Sequence.h
// for the requirements your design must meet.
// ============================================================================

#ifndef SEQUENCE_CPP
#define SEQUENCE_CPP

#include "Sequence.h"


// ---- Shared operations ----
//
// Write each of these using only the five primitives (and each other).

// Returns true if the sequence has no elements.
template <typename T>
bool Sequence<T>::empty() const {
    // TODO
    return size() == 0;
}

// Returns a reference to the first element.
// Throws std::out_of_range if the sequence is empty.
template <typename T>
T& Sequence<T>::front() {
    // TODO
    if( empty() ) {
        throw std::out_of_range("front() on empty sequence");
    }
        return at(0);
    
}

// Returns a reference to the last element.
// Throws std::out_of_range if the sequence is empty.
template <typename T>
T& Sequence<T>::back() {
    if (empty()) {
        throw std::out_of_range("back() on empty sequence");
    }
    return at(size() - 1);
}

// Appends a copy of value to the end.
template <typename T>
void Sequence<T>::push_back(const T& value) {
    // TODO
    
    insert (size(), value);

}

// Removes the last element.
// Throws std::out_of_range if the sequence is empty.
template <typename T>
void Sequence<T>::pop_back() {
    // TODO
    //throw std::logic_error("pop_back() not implemented");

    if (empty()) {
        throw std::out_of_range("pop_back() on empty sequence");

    }

    erase(size() -1);

}

// Returns true if some element compares equal (==) to value.
template <typename T>
bool Sequence<T>::contains(const T& value) const {
    // TODO
    //throw std::logic_error("contains() not implemented");

    return indexOf(value) != -1;

}

// Returns the index of the first element that compares equal (==) to value,
// or -1 if there is none.
template <typename T>
int Sequence<T>::indexOf(const T& value) const {
    for (std::size_t i = 0; i < size(); ++i) {
        if (at(i) == value) {
            return static_cast<int>(i);
        }
    }
    return -1;

}

// Writes the elements to os in this exact format, with no newline:
//     [1, 2, 3]
// An empty sequence prints [].
//
// Hint: os is an output stream, and you write to it exactly like std::cout:
// os << "[" or os << value. The caller chooses where the output goes, e.g.
// s.print(std::cout) prints to the terminal, and the tests pass a stream that
// collects the text so they can check it. LinkedList::print in LinkedList.cpp
// shows the idea (it walks nodes, which Sequence can't, but the os << parts
// are what you need).
template <typename T>
void Sequence<T>::print(std::ostream& os) const {

    os << "[";
    for (std::size_t i = 0; i < size(); ++i) {
        if (i > 0) {
            os << ", ";
        }
        os << at(i);
    }
    os << "]";

}

// Returns true if a and b have the same size and every pair of elements
// compares equal with ==. a and b may be different kinds of sequence, so a
// Vector can equal a LinkedList.
template <typename T>
bool operator==(const Sequence<T>& a, const Sequence<T>& b) {
    //compare sizes 
    if (a.size() != b.size()) {
        return false;
    }
    for (std::size_t i = 0; i < a.size(); ++i) {
        if (!(a.at(i) == b.at(i))) {
            return false;
        }
    }
    return true;
}

// Returns true if a and b are not equal.
template <typename T>
bool operator!=(const Sequence<T>& a, const Sequence<T>& b) {
    // TODO
    return !(a == b);}

#endif
