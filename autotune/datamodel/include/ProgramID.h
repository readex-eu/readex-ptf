#ifndef PROGRAMID_H_
#define PROGRAMID_H_

#include <string>

#include <boost/serialization/serialization.hpp>
#include <boost/serialization/string.hpp>

class ProgramID {
    std::string id;

    friend class boost::serialization::access;
    template<class Archive>
    void serialize( Archive& ar, const unsigned int version ) {
        ar& id;
    }

public:
    ProgramID() {
    }

    explicit ProgramID( std::string id_ ) : id( id_ ) {
    }

    bool operator==( const ProgramID& that ) const {
        return id == that.id;
    }

    bool operator!=( const ProgramID& that ) const {
        return id != that.id;
    }

    const std::string& toString() const {
        return id;
    }
};

#endif /* PROGRAMID_H_ */
