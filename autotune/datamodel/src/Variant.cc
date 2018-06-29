#include "Variant.h"

Variant::Variant() {
}

Variant::Variant( map<TuningParameter*, int> value ) {
    setValue( value );
}

map<TuningParameter*, int> Variant::getValue() const {
    return value;
}

void Variant::setValue( map<TuningParameter*, int> value ) {
    this->value = value;
}

bool Variant::operator==( const Variant& in ) const {
    if( value != in.value ) {
        // cout << "false in variant check\n";
        return false;
    }

    // cout << "true in variant check\n";
    return true;
}
bool Variant::operator!=( const Variant& in ) const {
    return !( *this == in );
}

string Variant::toString( int    indent,
                          string indentation_character ) {
    string                               base_indentation;
    map<TuningParameter*, int>::iterator parameter_iterator;
    for( int i = 0; i < indent; i++ ) {
        base_indentation.append( indentation_character );
    }
    stringstream temp;

    for( parameter_iterator = value.begin(); parameter_iterator != value.end(); parameter_iterator++ ) {
        temp << parameter_iterator->first->toString( indent, indentation_character ) << endl;
        temp << base_indentation << "Value: " << parameter_iterator->second << endl;
    }

    return temp.str();
}
