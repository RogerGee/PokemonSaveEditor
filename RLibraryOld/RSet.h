//RSet.h
#ifndef RSET_H
#define RSET_H
#include "RList.h"

namespace rtypes
{
    template<class T>
    class set
    {
    public:
        bool insert(const T& elem)
        {
            if (contains(elem))
                return false;
            _elements.add(elem);
            _elements.quicksort();
            return true;
        }
        bool remove(const T& elem)
        {
            dword ind = _contains(elem);
            if (ind<_elements.size())
            {
                _elements.remove_at(ind);
                return true;
            }
            return false;
        }
        bool contains(const T& elem) const
        {
            return _contains(elem)<_elements.size();
        }
        bool is_empty() const
        {
            return _elements.size()==0;
        }
        void empty()
        {
            _elements.empty();
        }
        dword size() const
        {
            return _elements.size();
        }
        // get sorted elements
        const T& operator [](dword i) const { return _elements[i]; }
    private:
        list<T> _elements;

        dword _contains(const T& elem) const
        {
            int beg = 0, end = (int) (_elements.size()-1);
            while (beg<=end)
            {
                dword index = (beg+end)/2;
                if (_elements[index]==elem)
                    return index;
                if (_elements[index]<elem)
                    beg = (int)index+1;
                else
                    end = (int)index-1;
            }
            return _elements.size();
        }
    };
}

#endif