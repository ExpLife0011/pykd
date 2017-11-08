#pragma once

#include <windows.h>

//#include "windbg.h"

namespace pykd {

///////////////////////////////////////////////////////////////////////////////

class PyThreadStateSaver {

public:

    PyThreadStateSaver( PyThreadState* s = NULL) {

        state = s;
    }

    ~PyThreadStateSaver() {
    }

    void saveState() {
        state = PyEval_SaveThread();
    }

    void restoreState() {
            if ( state )
                PyEval_RestoreThread( state );
    }

private:

    //DWORD   m_index;
    PyThreadState*      state;
};


//  --> call back 
//  { PyThread_StateSave  state( winext->getThreadState() );
//    do_callback();
//  }
//
//  ����  ��� ������ ������ �� ����� ����������� ������������ ���� ����� ������������ ��������,
//  � ����� �������� ����������,  ����� ��������� ���

class PyThread_StateSave {

public:

    PyThread_StateSave( PyThreadStateSaver  &threadState) :
        m_threadState( threadState )
    {
        m_threadState.restoreState();
    }

    ~PyThread_StateSave() {
        m_threadState.saveState();
    }

private:

    PyThreadStateSaver      &m_threadState;

};

// { PyThread_StateRestore   state;
//   long_or_block_opreration();
// }

class PyThread_StateRestore
{
public:

    PyThread_StateRestore(PyThreadStateSaver  &threadState) :
        m_threadState( threadState )
    {
        m_threadState.saveState();
    }

    ~PyThread_StateRestore() {
        m_threadState.restoreState();
    }
private:

    PyThreadStateSaver      &m_threadState;
};

/////////////////////////////////////////////////////////////////////////////////

}; //end namespace pykd