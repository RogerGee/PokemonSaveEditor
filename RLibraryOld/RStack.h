//RStack.h
#ifndef RSTACK_H
#define RSTACK_H
#include "RAllocator.h" // this gets RTypesTypes.h

namespace rtypes
{
    template<class T>
    class stack : protected rallocatorEx<T>
    {
    public:
        void push_back(const T& elem)
        {
            dword oldSize = _size();
            _virtAlloc(oldSize+1);  
            _getData()[oldSize] = elem;
        }
        T pop_back()
        {
            if (is_empty())
                return T();
            _virtAlloc(_size()-1);
            return _getData()[_size()];
        }
        T peek()
        {
            if (is_empty())
                return T();
            return _getData()[_size()-1];
        }
        void clear() // maintain capacity
        {
            _virtAlloc(0);
        }
        void reset() // reset capacity
        {
            _dealloc();
            // reset to default capacity
            _virtAlloc(1); // capacity should be 4
            _virtAlloc(0);
        }
        bool is_empty() const { return _size()==0; }
        dword size() const { return _size(); }
        dword capacity() const { return _allocationSize(); }
    protected:
        using rallocatorEx<T>::_allocationSize;
        using rallocatorEx<T>::_size;
        using rallocatorEx<T>::_virtAlloc;
        using rallocatorEx<T>::_getData;
        using rallocatorEx<T>::_dealloc;
    };
}

#endif