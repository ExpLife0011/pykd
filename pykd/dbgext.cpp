
#include "stdafx.h"

#include <dbgeng.h>

#include "win/dbgio.h"
#include "win/dbgeng.h"
#include "win/dbgpath.h"
#include "win/windbg.h"

using namespace pykd;

////////////////////////////////////////////////////////////////////////////////

extern "C" void initpykd();

////////////////////////////////////////////////////////////////////////////////

WindbgGlobalSession::WindbgGlobalSession()
{
    PyImport_AppendInittab("pykd", initpykd ); 

    PyEval_InitThreads();

    Py_Initialize();

    main = boost::python::import("__main__");

    python::object   main_namespace = main.attr("__dict__");

    // ������ ������ from pykd import *
    python::object   pykd = boost::python::import( "pykd" );

    python::dict     pykd_namespace( pykd.attr("__dict__") ); 

    python::list     iterkeys( pykd_namespace.iterkeys() );

    for (int i = 0; i < boost::python::len(iterkeys); i++)
    {
        std::string     key = boost::python::extract<std::string>(iterkeys[i]);

        main_namespace[ key ] = pykd_namespace[ key ];
    }

    pyState = PyEval_SaveThread();
}
volatile LONG            WindbgGlobalSession::sessionCount = 0;

WindbgGlobalSession     *WindbgGlobalSession::windbgGlobalSession = NULL; 

////////////////////////////////////////////////////////////////////////////////

BOOL WINAPI DllMain(
  __in  HINSTANCE /*hinstDLL*/,
  __in  DWORD fdwReason,
  __in  LPVOID /*lpvReserved*/
)
{
    switch (fdwReason)
    {
    case DLL_PROCESS_ATTACH:
        CoInitialize(NULL);
        break;

    case DLL_PROCESS_DETACH:
        CoUninitialize();
        break;
    }
    return TRUE;
}

////////////////////////////////////////////////////////////////////////////////

HRESULT
CALLBACK
DebugExtensionInitialize(
    OUT PULONG  Version,
    OUT PULONG  Flags )
{
    *Version = DEBUG_EXTENSION_VERSION( 1, 0 );
    *Flags = 0;

    WindbgGlobalSession::StartWindbgSession();

    return S_OK;
}

////////////////////////////////////////////////////////////////////////////////

VOID
CALLBACK
DebugExtensionUninitialize()
{
    WindbgGlobalSession::StopWindbgSession();
}

////////////////////////////////////////////////////////////////////////////////

HRESULT 
CALLBACK
py( PDEBUG_CLIENT4 client, PCSTR args )
{

    WindbgGlobalSession::RestorePyState();

    PyThreadState   *globalInterpreter = PyThreadState_Swap( NULL );
    PyThreadState   *localInterpreter = Py_NewInterpreter();

    try {

        // �������� ������ � ����������� ���� ( ����� ��� ������ exec_file )
        python::object       main =  python::import("__main__");

        python::object       global(main.attr("__dict__"));

        // ����������� ����/����� ( ����� � ������� ����� ���� ������ print )

        python::object       sys = python::import("sys");

        sys.attr("stdout") = python::object( DbgOut() );
        sys.attr("stderr") = python::object( DbgErr() );
        sys.attr("stdin") = python::object( DbgIn() );

        // ����������� ������ ��������� ���������� ( ����� ��� ������ traceback � )
        python::object       tracebackModule = python::import("traceback");

        // ������ ����������
        typedef  boost::escaped_list_separator<char>    char_separator_t;
        typedef  boost::tokenizer< char_separator_t >   char_tokenizer_t;  

        std::string                 argsStr( args );

        char_tokenizer_t            token( argsStr , char_separator_t( "", " \t", "\"" ) );
        std::vector<std::string>    argsList;

        for ( char_tokenizer_t::iterator   it = token.begin(); it != token.end(); ++it )
        {
            if ( *it != "" )
                argsList.push_back( *it );
        }            
            
        if ( argsList.size() == 0 )
            return S_OK;      
            
        char    **pythonArgs = new char* [ argsList.size() ];
     
        for ( size_t  i = 0; i < argsList.size(); ++i )
            pythonArgs[i] = const_cast<char*>( argsList[i].c_str() );

        PySys_SetArgv( (int)argsList.size(), pythonArgs );

        delete[]  pythonArgs;

       // ����� ���� � �����
        std::string     fullScriptName;
        DbgPythonPath   dbgPythonPath;
        
        if ( !dbgPythonPath.getFullFileName( argsList[0], fullScriptName ) )
        {
            eprintln( L"script file not found" );
        }
        else
        try {

            python::object       result;

            result =  python::exec_file( fullScriptName.c_str(), global, global );
        }
        catch( boost::python::error_already_set const & )
        {
            // ������ � �������
            PyObject    *errtype = NULL, *errvalue = NULL, *traceback = NULL;

            PyErr_Fetch( &errtype, &errvalue, &traceback );

            PyErr_NormalizeException( &errtype, &errvalue, &traceback );

            std::wstringstream       sstr;

            python::object   lst = 
                python::object( tracebackModule.attr("format_exception" ) )( 
                    python::handle<>( errtype ),
                    python::handle<>( python::allow_null( errvalue ) ),
                    python::handle<>( python::allow_null( traceback ) ) );

            sstr << std::endl << std::endl;

            for ( long i = 0; i < python::len(lst); ++i )
                sstr << std::wstring( python::extract<std::wstring>(lst[i]) ) << std::endl;

            eprintln( sstr.str() );
        }

    }
    catch(...)
    {
        eprintln( L"unexpected error" );
    }

    Py_EndInterpreter( localInterpreter ); 
    PyThreadState_Swap( globalInterpreter );

    WindbgGlobalSession::SavePyState();

    return S_OK;
}

////////////////////////////////////////////////////////////////////////////////

HRESULT 
CALLBACK
pycmd( PDEBUG_CLIENT4 client, PCSTR args )
{
    WindbgGlobalSession::RestorePyState();

    ULONG    mask = 0;
    client->GetOutputMask( &mask );
    client->SetOutputMask( mask & ~DEBUG_OUTPUT_PROMPT ); // ������ ��� �����

    try {

        // ��������������� ����������� ������� ��
        python::object       sys = python::import("sys");
       
        sys.attr("stdout") = python::object( DbgOut() );
        sys.attr("stderr") = python::object( DbgErr() );
        sys.attr("stdin") = python::object( DbgIn() );

        PyRun_String(
            "__import__('code').InteractiveConsole(__import__('__main__').__dict__).interact()", 
            Py_file_input,
            WindbgGlobalSession::global().ptr(),
            WindbgGlobalSession::global().ptr()
            );

        // ����� �� �������������� ���������� ����� ���������� raise SystemExit(code)
        // ������� ����� ����� �������� ���������� callback ��
        PyErr_Clear();
    }
    catch(...)
    {
        eprintln( L"unexpected error" );
    }

    client->SetOutputMask( mask );

    WindbgGlobalSession::SavePyState();

    return S_OK;
}

////////////////////////////////////////////////////////////////////////////////
