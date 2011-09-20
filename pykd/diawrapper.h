
#pragma once

#include <cvconst.h>
#include "dbgexcept.h"

namespace pyDia {

typedef CComPtr< IDiaSymbol > DiaSymbolPtr;
typedef CComPtr< IDiaEnumSymbols > DiaEnumSymbolsPtr;
typedef CComPtr< IDiaDataSource > DiaDataSourcePtr;
typedef CComPtr< IDiaSession > DiaSessionPtr;

////////////////////////////////////////////////////////////////////////////////
// DIA Exceptions
////////////////////////////////////////////////////////////////////////////////
class Exception : public pykd::DbgException {
public:
    Exception(const std::string &desc, HRESULT hres)
        : DbgException( makeFullDesc(desc, hres) )
        , m_hres(hres)
    {
    }

    Exception(const std::string &desc)
        : DbgException(descPrefix + desc)
        , m_hres(S_FALSE)
    {
    }

    HRESULT getRes() const {
        return m_hres;
    }

    static void exceptionTranslate(const Exception &e);

    static void setTypeObject(PyObject *p) {
        diaExceptTypeObject = p;
    }

private:

    static const std::string descPrefix;

    static PyObject *diaExceptTypeObject;

    static std::string makeFullDesc(const std::string &desc, HRESULT hres);

    HRESULT m_hres;
};

////////////////////////////////////////////////////////////////////////////////
// Symbol
////////////////////////////////////////////////////////////////////////////////
class Symbol {
public:
    Symbol() {}

    python::list findChildrenImpl(
        ULONG symTag,
        const std::string &name,
        DWORD nameCmpFlags
    );

    python::list findChildren(
        const std::string &name
    )
    {
        return findChildrenImpl(SymTagNull, name, nsfCaseSensitive);
    }

    ULONGLONG getSize();

    std::string getName();

    python::object getType();

    python::object getIndexType();

    ULONG getSymTag();

    ULONG getRva();

    ULONG getLocType();

    python::object getValue();

    bool isBasicType();

    ULONG getBaseType();

    ULONG getBitPosition();

    python::object getChildByName(const std::string &_name);
    ULONG getChildCount();
    python::object getChildByIndex(ULONG _index);

    std::string print();

public:
    typedef std::pair<ULONG, const char *> ValueNameEntry;

    static const ValueNameEntry symTagName[SymTagMax];
    static const ValueNameEntry locTypeName[LocTypeMax];
    static const ValueNameEntry basicTypeName[];
    static const size_t cntBasicTypeName;

protected:

    void throwIfNull(const char *desc)
    {
        if (!m_symbol)
            throw Exception(std::string(desc) + " failed, DIA object is not initialized");
    }

    Symbol(__inout DiaSymbolPtr &_symbol) {
        m_symbol = _symbol.Detach();
    }

    DiaSymbolPtr m_symbol;
};

////////////////////////////////////////////////////////////////////////////////
// Global scope: source + sessions
////////////////////////////////////////////////////////////////////////////////
class GlobalScope : public Symbol {
public:
    GlobalScope() {}

    // create GlobalScope instance
    static python::object openPdb(const std::string &filePath);

private:

    GlobalScope(
        __inout DiaDataSourcePtr &_scope,
        __inout DiaSessionPtr &_session,
        __inout DiaSymbolPtr &_globalScope
    );

    DiaDataSourcePtr m_source;
    DiaSessionPtr m_session;
};

////////////////////////////////////////////////////////////////////////////////

};
