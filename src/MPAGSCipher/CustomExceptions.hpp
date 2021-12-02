#ifndef MPAGSCIPHER_CUSTOMEXCEPTIONS_HPP
#define MPAGSCIPHER_CUSTOMEXCEPTIONS_HPP

#include <exception>
#include <string>
#include <stdexcept>

/**
 * \file CustomExceptions.hpp
 * \brief Contains the declaration of various custom exception classes
 */

/**
 * \class MissingArgument
 * \brief Informs the user when they forget to add an argument to an option
 */

class MissingArgument : public std::invalid_argument {
public:
MissingArgument( const std::string& msg ) : std::invalid_argument(msg)
{
}

};

class UnknownArgument : public std::invalid_argument {
public:
explicit UnknownArgument( const std::string& msg ) : std::invalid_argument(msg)
{
}    
};

class InvalidKey : public std::invalid_argument {
public:
InvalidKey( const std::string& msg ) : std::invalid_argument(msg)
{
}
};

class InvalidCipher : public std::invalid_argument {
public:
InvalidCipher( const std::string& msg ) : std::invalid_argument(msg)
{
}
};


#endif