
#include "stdafx.h"

#include "pycpucontext.h"
#include "variant.h"


namespace pykd {

///////////////////////////////////////////////////////////////////////////////

python::object getRegisterByName( const std::wstring &name )
{
    kdlib::NumVariant var;

    do {

        AutoRestorePyState  pystate;

        var = kdlib::getRegisterByName(name);
    
    } while(false);

    return NumVariantAdaptor::convertToPython( var );
}

///////////////////////////////////////////////////////////////////////////////

python::object getRegisterByIndex(unsigned long index)
{
    kdlib::NumVariant var;

    do {

        AutoRestorePyState  pystate;

        var = kdlib::getRegisterByIndex(index);
    
    } while(false);

    return NumVariantAdaptor::convertToPython( var );
}

///////////////////////////////////////////////////////////////////////////////

std::wstring getRegisterNameByIndex(unsigned long index)
{
    AutoRestorePyState  pystate;
    return kdlib::getRegisterName(index);
}

///////////////////////////////////////////////////////////////////////////////

python::list getCurrentStack()
{
    kdlib::StackPtr  stack;
    unsigned long  numberFrames;

    do {
        AutoRestorePyState  pystate;
        stack = kdlib::getStack();
        numberFrames = stack->getFrameCount();
    } while(false);

    python::list  lst;
    for ( unsigned long  i = 0; i < numberFrames; ++i )
        lst.append( stack->getFrame(i) );

    return lst;
}

///////////////////////////////////////////////////////////////////////////////

std::wstring StackFrameAdapter::print( kdlib::StackFramePtr& frame )
{
    AutoRestorePyState  pystate;

    std::wstringstream sstr;
    sstr << L"Frame: ";
    sstr << L"IP=" << std::hex << frame->getIP() << L"  ";
    sstr << L"Return=" << std::hex << frame->getRET() << L"  ";
    sstr << L"Frame Offset=" << std::hex << frame->getFP() << L"  ";
    sstr << L"Stack Offset=" << std::hex << frame->getSP();

    return sstr.str();
}
///////////////////////////////////////////////////////////////////////////////

python::list StackFrameAdapter::getParamsList( kdlib::StackFramePtr&  frame)
{
    typedef std::vector< std::pair< std::wstring, kdlib::TypedVarPtr> >  FuncParamList;

    FuncParamList  paramLst;
    unsigned long  paramCount;

    do {
        AutoRestorePyState  pystate;
        paramCount = frame->getTypedParamCount();
        for ( unsigned long i = 0; i < paramCount; ++i )
        {
            kdlib::TypedVarPtr  param = frame->getTypedParam(i);
            std::wstring  paramName =   frame->getTypedParamName(i);

            paramLst.push_back( std::make_pair( paramName, param) );
        }
    } while(false);

    python::list  pyLst;

    for ( unsigned long i = 0; i < paramCount; ++i )
        pyLst.append( python::make_tuple( paramLst[i].first, paramLst[i].second ) );

    return pyLst;
}

///////////////////////////////////////////////////////////////////////////////

python::dict StackFrameAdapter::getParamsDict( kdlib::StackFramePtr&  frame)
{
    typedef std::vector< std::pair< std::wstring, kdlib::TypedVarPtr> >  FuncParamList;

    FuncParamList  paramLst;
    unsigned long  paramCount;

    do {
        AutoRestorePyState  pystate;
        paramCount = frame->getTypedParamCount();
        for ( unsigned long i = 0; i < paramCount; ++i )
        {
            kdlib::TypedVarPtr  param = frame->getTypedParam(i);
            std::wstring  paramName =   frame->getTypedParamName(i);

            paramLst.push_back( std::make_pair( paramName, param) );
        }
    } while(false);

    python::dict  pyLst;

    for ( unsigned long i = 0; i < paramCount; ++i )
        pyLst[paramLst[i].first] = paramLst[i].second;

    return pyLst;
}

///////////////////////////////////////////////////////////////////////////////

python::list StackFrameAdapter::getLocalsList(kdlib::StackFramePtr& frame)
{
    typedef std::vector< std::pair< std::wstring, kdlib::TypedVarPtr> >  LocalVarList;

    LocalVarList  localLst;
    unsigned long  localCount;

    do {
        AutoRestorePyState  pystate;
        localCount = frame->getLocalVarCount();
        for ( unsigned long i = 0; i < localCount; ++i )
        {
            kdlib::TypedVarPtr  param = frame->getLocalVar(i);
            std::wstring  paramName =  frame->getLocalVarName(i);

            localLst.push_back( std::make_pair( paramName, param) );
        }
    } while(false);

    python::list  pyLst;

    for ( unsigned long i = 0; i < localCount; ++i )
        pyLst.append( python::make_tuple( localLst[i].first, localLst[i].second ) );

    return pyLst;
}

///////////////////////////////////////////////////////////////////////////////

python::dict StackFrameAdapter::getLocalsDict(kdlib::StackFramePtr& frame)
{
    typedef std::vector< std::pair< std::wstring, kdlib::TypedVarPtr> >  LocalVarList;

    LocalVarList  localLst;
    unsigned long  localCount;

    do {
        AutoRestorePyState  pystate;
        localCount = frame->getLocalVarCount();
        for ( unsigned long i = 0; i < localCount; ++i )
        {
            kdlib::TypedVarPtr  param = frame->getLocalVar(i);
            std::wstring  paramName =  frame->getLocalVarName(i);

            localLst.push_back( std::make_pair( paramName, param) );
        }
    } while(false);

    python::dict  pyLst;

    for ( unsigned long i = 0; i < localCount; ++i )
        pyLst[localLst[i].first] = localLst[i].second;

    return pyLst;
}

///////////////////////////////////////////////////////////////////////////////

python::tuple CPUContextAdapter::getRegisterByIndex(unsigned long index)
{

    kdlib::NumVariant var;
    std::wstring  name;
    
    do {

        AutoRestorePyState  pystate;

        if (index >= kdlib::getRegisterNumber())
            throw kdlib::IndexException(index);

        name = kdlib::getRegisterName(index);
        
        var = kdlib::getRegisterByIndex(index);

    } while (false);

    return python::make_tuple(name, NumVariantAdaptor::convertToPython(var));
}

///////////////////////////////////////////////////////////////////////////////

} // end namespace pykd
