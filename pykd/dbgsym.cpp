#include "stdafx.h"

#include "dbgext.h"
#include "dbgsym.h"
#include "dbgexcept.h"
#include "dbgprint.h"

/////////////////////////////////////////////////////////////////////////////////

boost::python::object
findSymbolForAddress( ULONG64 addr )
{
    HRESULT         hres;  
    
    try {
    
        DEBUG_MODULE_AND_ID     debugId;
        ULONG64                 displace = 0;
        
        if ( *( (ULONG*)&addr + 1 ) == 0 )
            *( (ULONG*)&addr + 1 ) = 0xFFFFFFFF;        
            
        ULONG     moduleIndex;
        ULONG64   moduleBase;            
        hres = dbgExt->symbols->GetModuleByOffset( addr, 0, &moduleIndex, &moduleBase );
    
        if ( FAILED( hres ) )
        {
            return boost::python::object( "out of module" );   
        }         
        
        char   moduleName[0x100];
        hres =  dbgExt->symbols2->GetModuleNameString( DEBUG_MODNAME_MODULE, moduleIndex, moduleBase, 
                    moduleName, sizeof( moduleName ), NULL );
                    
        if ( FAILED( hres ) )
             throw DbgException( "IDebugSymbol2::GetModuleNameString  failed" );           
    
        ULONG   entries = 0;
        hres =  dbgExt->symbols3->GetSymbolEntriesByOffset( addr, 0, &debugId, &displace, 1, &entries );
        if ( FAILED( hres ) )
             throw DbgException( "IDebugSymbol3::GetSymbolEntriesByOffset  failed" );
             
        std::stringstream      ss;             
             
        if ( entries == 0 )
        {
            ss << moduleName << "+" << std::hex << ( addr - moduleBase );
            return boost::python::object( ss.str() );             
        }            
                          
        char      symbolName[0x100];             
        hres =  dbgExt->symbols3->GetSymbolEntryString( &debugId, 0, symbolName, sizeof(symbolName ), NULL );   
        if ( FAILED( hres ) )
             throw DbgException( "IDebugSymbol3::GetSymbolEntryString  failed" );  
             
        ss << moduleName << "!" << symbolName;
        return boost::python::object( ss.str() );                                
            
    }
	catch( std::exception  &e )
	{
		dbgExt->control->Output( DEBUG_OUTPUT_ERROR, "pykd error: %s\n", e.what() );
	}
	catch(...)
	{
		dbgExt->control->Output( DEBUG_OUTPUT_ERROR, "pykd unexpected error\n" );
	}	 
	
	return boost::python::object( addr );
}

/////////////////////////////////////////////////////////////////////////////////

ULONG64
findAddressForSymbol( const std::string  &moduleName, const std::string  &symbolName )
{
    HRESULT         hres;  
    
    try {
    
        std::string   ModuleSymName = moduleName;
        ModuleSymName += "!";
        ModuleSymName += symbolName;
    
        ULONG64    offset = 0ULL;
        hres = dbgExt->symbols->GetOffsetByName( ModuleSymName.c_str(), &offset );
        if ( FAILED( hres ) )
             throw DbgException( "IDebugSymbol::GetOffsetByName  failed" );                
        
        return offset;    
    }
    catch( std::exception  &e )
	{
		dbgExt->control->Output( DEBUG_OUTPUT_ERROR, "pykd error: %s\n", e.what() );
	}
	catch(...)
	{
		dbgExt->control->Output( DEBUG_OUTPUT_ERROR, "pykd unexpected error\n" );
	}	 
	
	return (ULONG64)~0;
}

/////////////////////////////////////////////////////////////////////////////////