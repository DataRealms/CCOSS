//--------------------------------------------------------------------------------------------------------------------
// SMART POINTER TEMPLATE
// written by Frederic My
// fmy@fairyengine.com
//
// smart pointer to T : T must inherit from RefCounted
//--------------------------------------------------------------------------------------------------------------------

#ifndef _SMARTPOINTER_H_
#define _SMARTPOINTER_H_

//----------------------------------------------- CLASSES ------------------------------------------------------------

//--------------------------------------------------------------------------------------------------------------------
// SmartPointer
//--------------------------------------------------------------------------------------------------------------------

namespace RTE
{

template <class CObj>
class SmartPointer
{
// public methods
    public:
                    SmartPointer        (CObj *pObj=NULL);
                    SmartPointer        (const SmartPointer &Smart);
                   ~SmartPointer        (void);

// operators
    SmartPointer &  operator =          (const SmartPointer &Smart);
    SmartPointer &  operator =          (CObj *pObj);

    CObj *          operator ->         (void) const;
    CObj &          operator *          (void) const;
    operator        CObj *              (void) const;

    bool            operator ==         (CObj *pObj) const;
    bool            operator !=         (CObj *pObj) const;
    bool            operator ==         (const SmartPointer &Smart) const;
    bool            operator !=         (const SmartPointer &Smart) const;

// dumb pointer
    CObj *          GetDumb             (void) const;

// protected data
    protected:

// pointed object
    CObj *          m_pDumbPtr;
};

}

//----------------------------------------------- MACROS -------------------------------------------------------------

// SMART_POINTER(Class); => defines the ClassSP type (smart pointer to an object of class 'Class')

#define   SMART_POINTER(Class)    \
    class   Class;                  \
    typedef RTE::SmartPointer<Class> Class##SP;

//----------------------------------------------- INLINES ------------------------------------------------------------

#include  "SmartPointer.inl"

//--------------------------------------------------------------------------------------------------------------------

#endif // _SMARTPOINTER_H_
