//--------------------------------------------------------------------------------------------------------------------
// SMART POINTER TEMPLATE
// written by Frederic My
// fmy@fairyengine.com
//--------------------------------------------------------------------------------------------------------------------

#ifndef _SMARTPOINTER_INL_
#define _SMARTPOINTER_INL_

#ifdef    _DEBUG
#define   INLINE
#else
#define   INLINE    inline
#endif

namespace RTE
{

//--------------------------------------------------------------------------------------------------------------------
// CONSTRUCTORS / DESTRUCTOR
//--------------------------------------------------------------------------------------------------------------------

template <class CObj> inline SmartPointer<CObj>::SmartPointer(CObj *pObj)
{
    m_pDumbPtr = pObj;
    if (m_pDumbPtr)
        m_pDumbPtr->AddRef();
}


template <class CObj> inline SmartPointer<CObj>::SmartPointer(const SmartPointer &Smart)
{
    m_pDumbPtr = Smart.m_pDumbPtr;
    if (m_pDumbPtr)
        m_pDumbPtr->AddRef();
}


template <class CObj> inline SmartPointer<CObj>::~SmartPointer()
{
    if (m_pDumbPtr)
        m_pDumbPtr->Release();
}

//--------------------------------------------------------------------------------------------------------------------
// OPERATORS
//--------------------------------------------------------------------------------------------------------------------

template <class CObj> inline SmartPointer<CObj>& SmartPointer<CObj>::operator = (const SmartPointer &Smart)
{
    if (m_pDumbPtr != Smart.m_pDumbPtr) // check assignment to self
    {
        if (m_pDumbPtr)
                m_pDumbPtr->Release();

        m_pDumbPtr = Smart.m_pDumbPtr;
        if (m_pDumbPtr)
                m_pDumbPtr->AddRef();
    }

    return *this;
}


template <class CObj> inline SmartPointer<CObj>& SmartPointer<CObj>::operator = (CObj *pObj)
{
    if (m_pDumbPtr != pObj) // check assignment to self
    {
        if (m_pDumbPtr)
                m_pDumbPtr->Release();

        m_pDumbPtr = pObj;
        if (m_pDumbPtr)
                m_pDumbPtr->AddRef();
    }

    return *this;
}


template <class CObj> inline CObj *SmartPointer<CObj>::operator -> () const
{
    return m_pDumbPtr;
}


template <class CObj> inline CObj &SmartPointer<CObj>::operator * () const
{
    return *m_pDumbPtr;
}


template <class CObj> inline SmartPointer<CObj>::operator CObj *() const
{
    return m_pDumbPtr;
}


template <class CObj> inline bool SmartPointer<CObj>::operator == (CObj *pObj) const
{
    return(m_pDumbPtr == pObj);
}


template <class CObj> inline bool SmartPointer<CObj>::operator != (CObj *pObj) const
{
    return(m_pDumbPtr != pObj);
}


template <class CObj> inline bool SmartPointer<CObj>::operator == (const SmartPointer &Smart) const
{
    return(m_pDumbPtr == Smart.m_pDumbPtr);
}


template <class CObj> inline bool SmartPointer<CObj>::operator != (const SmartPointer &Smart) const
{
    return(m_pDumbPtr != Smart.m_pDumbPtr);
}

// this can sometimes be useful to cast the dumb pointer :

template <class CObj> inline CObj *SmartPointer<CObj>::GetDumb() const
{
    return m_pDumbPtr;
}

} // namespace

#endif // _SMARTPOINTER_INL_
