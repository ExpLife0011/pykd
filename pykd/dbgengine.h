#pragma once

namespace pykd {

// manage debug target
ULONG startProcess( const std::wstring  &processName );
void detachProcess( ULONG processId = -1);
void terminateProcess( ULONG processId = -1);

void debugGo();

//manage debug module
ULONG64 findModuleBase( const std::string &moduleName );
ULONG64 findModuleBase( ULONG64 offset );
std::string getModuleName( ULONG64 baseOffset );

//manage access to target memory
ULONG64 addr64( ULONG64 offset );

};

