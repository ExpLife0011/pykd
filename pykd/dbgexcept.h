#pragma once

#include <exception>
#include <string>

#include "kdlib/exceptions.h"

///////////////////////////////////////////////////////////////////////////////

namespace pykd {


template< class TExcept >
struct exceptPyType{
    static python::handle<>     pyExceptType;
};

template< class TExcept, class TBaseExcept = python::detail::not_specified >
class exception  {

public:

    exception( const std::string& className, const std::string& classDesc ) 
    {
        python::handle<>   basedtype;      

        if ( boost::is_same<TBaseExcept, python::detail::not_specified>::value )
        {
            basedtype = python::handle<>(PyExc_Exception);
        }
        else
        {
            basedtype = exceptPyType<TBaseExcept>::pyExceptType;
        }

        python::dict       ob_dict;
       
        ob_dict["__doc__"] = classDesc;

        python::tuple      ob_bases = python::make_tuple( basedtype );

        python::object     ob = python::object( python::handle<>(Py_TYPE(basedtype.get()) ) )( className, ob_bases, ob_dict );

        python::scope().attr( className.c_str() ) = ob;

        exceptPyType<TExcept>::pyExceptType = python::handle<>( ob.ptr() );
    }
};



inline void exceptionTranslate(const kdlib::DbgException &e ) 
{
    if ( typeid(e).hash_code() == typeid(kdlib::MemoryException).hash_code() )
    {
        python::object      exceptObj = python::object( exceptPyType<kdlib::MemoryException>::pyExceptType )( e.what() );
        PyErr_SetObject( exceptPyType<kdlib::MemoryException>::pyExceptType.get(), exceptObj.ptr());
        return;
    }

    if ( typeid(e).hash_code() == typeid(kdlib::SymbolException).hash_code() )
    {
        python::object      exceptObj = python::object( exceptPyType<kdlib::SymbolException>::pyExceptType )( e.what() );
        PyErr_SetObject( exceptPyType<kdlib::SymbolException>::pyExceptType.get(), exceptObj.ptr());
        return;
    }

    if ( typeid(e).hash_code() == typeid(kdlib::TypeException).hash_code() )
    {
        python::object      exceptObj = python::object( exceptPyType<kdlib::TypeException>::pyExceptType )( e.what() );
        PyErr_SetObject( exceptPyType<kdlib::TypeException>::pyExceptType.get(), exceptObj.ptr());
        return;
    }

    if ( typeid(e).hash_code() == typeid(kdlib::IndexException).hash_code() )
    {
        PyErr_SetString( PyExc_IndexError, "Index out of range");
        return;
    }

    python::object      exceptObj = python::object( exceptPyType<kdlib::DbgException>::pyExceptType )( e.what() );
    PyErr_SetObject( exceptPyType<kdlib::DbgException>::pyExceptType.get(), exceptObj.ptr());
}

inline void registerExceptions()
{
    pykd::exception<kdlib::DbgException>( "DbgException", "Pykd base exception class" );
    pykd::exception<kdlib::MemoryException,kdlib::DbgException>( "MemoryException", "Target memory access exception class" );
    pykd::exception<kdlib::SymbolException,kdlib::DbgException>( "SymbolException", "Symbol exception" );
    pykd::exception<kdlib::TypeException,kdlib::SymbolException>( "TypeException", "type exception" );

    python::register_exception_translator<kdlib::DbgException>( &exceptionTranslate );
}

/////////////////////////////////////////////////////////////////////////////////////

void printException();

///////////////////////////////////////////////////////////////////////////////////

}; // namespace pykd

