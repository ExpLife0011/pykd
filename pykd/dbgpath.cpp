#include "stdafx.h"

#include "dbgpath.h"

#include <boost/tokenizer.hpp>
#include <boost/python.hpp>

///////////////////////////////////////////////////////////////////////////////



DbgPythonPath::DbgPythonPath()
{
    DWORD   enviromentSize = 0;
    
    enviromentSize = GetEnvironmentVariableA( "PYTHONPATH", NULL, enviromentSize );

    char   *enviromentBuffer = new char[ enviromentSize ];
    
    GetEnvironmentVariableA( "PYTHONPATH", enviromentBuffer, enviromentSize );

    typedef  boost::escaped_list_separator<char>    char_separator_t;
    typedef  boost::tokenizer< char_separator_t >   char_tokenizer_t;  
    
    std::string     pytonPath( enviromentBuffer );
  
    char_tokenizer_t            token( pytonPath, char_separator_t( "", "; \t", "\"" ) );

    for ( char_tokenizer_t::iterator   it = token.begin(); it != token.end(); ++it )
    {
        if ( *it != "" )
            m_pathList.push_back( *it );
    }        
    
    delete[] enviromentBuffer;
}

///////////////////////////////////////////////////////////////////////////////

std::string 
DbgPythonPath::getStr() const
{
    std::string     str;
    std::vector<std::string>::const_iterator      it = m_pathList.begin();
    
    for ( ; it != m_pathList.end(); ++it )
    {
        str += *it;
        str += ";";
    }
    
    return str;
}
 
///////////////////////////////////////////////////////////////////////////////
 
bool
DbgPythonPath::findPath( 
    const std::string  &fileName,
    std::string  &fullFileName,
    std::string  &filePath ) const
{
    std::vector< std::string >      extPathList;

    boost::python::object       sys = boost::python::import( "sys");

    boost::python::list       pathList( sys.attr("path") );
    
    boost::python::ssize_t n = boost::python::len(pathList);
    for(boost::python::ssize_t i=0;i<n;i++) 
         extPathList.push_back(  boost::python::extract<std::string>( pathList[i] ) );

    bool    pyExt = fileName.rfind( ".py" ) ==  fileName.length() - 3; 

    // 1. ���� � ������� ����������
    DWORD   bufSize =
        SearchPathA(
            NULL,
            fileName.c_str(),
            pyExt ? NULL : ".py",
            0,
            NULL,
            NULL );  
            
    if ( bufSize > 0 )    
    {
        char    *fullFileNameCStr = new char[ bufSize ];
        char    *partFileNameCStr = NULL;

        SearchPathA(
            NULL,
            fileName.c_str(),
            pyExt ? NULL : ".py",
            bufSize,
            fullFileNameCStr,
            &partFileNameCStr );              

        fullFileName = std::string( fullFileNameCStr );
        filePath = std::string( fullFileNameCStr, partFileNameCStr );

        delete[] fullFileNameCStr;

        return true;        
    }                
            
    // 2. ���� �� ���� �����������, ��������� � m_pathList
    
    std::vector<std::string>::const_iterator      it = extPathList.begin();
    
    for ( ; it != extPathList.end(); ++it )
    {
        DWORD   bufSize =
            SearchPathA(
                (*it).c_str(),
                fileName.c_str(),
                 pyExt ? NULL : ".py",
                0,
                NULL,
                NULL );          
                
        if ( bufSize > 0 )
        {
            char    *fullFileNameCStr = new char[ bufSize ];
            char    *partFileNameCStr = NULL;
            
            SearchPathA(
                (*it).c_str(),
                fileName.c_str(),
                pyExt ? NULL : ".py",
                bufSize,
                fullFileNameCStr,
                &partFileNameCStr );              
            
            fullFileName = std::string( fullFileNameCStr );
            filePath = std::string( fullFileNameCStr, partFileNameCStr );
            
            delete[] fullFileNameCStr;
            
            return true;
        }                  
    }
    
    return false;
}
    
//////////////////////////////////////////////////////////////////////////////    
