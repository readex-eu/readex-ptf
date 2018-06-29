#include "Constraint.h"

list<int> Constraint::getElements() {
    return elements;
}

int Constraint::getLeftBoundary() {
    return leftBoundary;
}

int Constraint::getRightBoundary() {
    return rightBoundary;
}

void Constraint::setElements( list<int> elements ) {
    this->elements = elements;
}

void Constraint::setLeftBoundary( int leftBoundary ) {
    this->leftBoundary = leftBoundary;
}

void Constraint::setRightBoundary( int rightBoundary ) {
    this->rightBoundary = rightBoundary;
}
