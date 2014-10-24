//IrregArray.h
#ifndef IRREGARRAY_H
#define IRREGARRAY_H
#include "RAllocator.h" // gets RTypesTypes.h

namespace rtypes
{
    template<class T>
    struct irregular_array_element
    {
        T data;
        dword gap;
    };

/*
        An irregular array treats the distance between elements as being irregular. Basically, an irregular array has two types of indexing,
    actual and virtual. When using the [] operators, a virtual index is expected. To get the virtual index from an actual index, use the
    get_virtual_index method; vice versa with get_actual_index.
    
    Given a normal array of integers: 1,2,3,4,5
        imagine an application where 3 would never need to be used. However, the index values for
        4 and 5 need to remain the same (for consistency). An irregular array would hold 1,2,4,5
        but note the gap left by the element 3, which is not stored.
    code ex.
        irregular_array<int> a;
        a.push_back(1);
        a.push_back(2,1); // gap of 1 for the missing element 3
        a.push_back(4);
        a.push_back(5);

        int i1 = a[2]; // whoops! there is no such element at a[2]
        // i1 equals a default int
        bool doesExists = a.virtual_index_exists(2); // doesExist == false

        int i2 = a[3]; // i2 == 4

*/

    template<class T>
    class irregular_array : protected rallocatorEx< irregular_array_element<T> >
    {
    public:
        irregular_array()
        {
            _virtualSize = 0;
        }
        bool virtual_index_exists(dword vIndex) const
        {
            _Element* data = _getData();
            dword cnt = 0, actual = 0;
            while (cnt<vIndex && actual<_size())
                cnt += 1+data[actual++].gap;
            return cnt==vIndex;
        }
        bool actual_index_exists(dword aIndex) const
        {
            return aIndex<_size();
        }
        dword get_actual_index(dword vIndex) const
        {
            _Element* data = _getData();
            dword cnt = 0, actual = 0;
            while (cnt<vIndex && actual<_size())
                cnt += data[actual++].gap+1;
            return actual;
        }
        dword get_virtual_index(dword aIndex) const
        {
            _Element* data = _getData();
            dword vIndex = 0, actual = 0;
            while (actual<aIndex && actual<_size())
                vIndex += 1+data[actual++].gap;
            return vIndex;
        }
        T& operator [](dword i)
        {
            _Element* data = _getData();
            dword actual, cnt = 0;
            for (actual = 0;actual<_size();actual++)
            {
                if (cnt>=i)
                    return data[actual].data;
                cnt += data[actual].gap+1;
            }
            return data[actual].data;
        }
        const T& operator [](dword i) const
        {
            _Element* data = _getData();
            dword actual, cnt = 0;
            for (actual = 0;actual<_size();actual++)
            {
                if (cnt>=i)
                    return data[actual].data;
                cnt += data[actual].gap+1;
            }
            return data[actual].data;
        }
        void push_back(const T& elem,dword gap = 0)
        {
            dword oldSize = _size();
            _Element* data;
            _virtAlloc(oldSize+1);
            data = _getData(); // after a potential allocation
            data[oldSize].data = elem;
            data[oldSize].gap = gap;
            _virtualSize += 1+gap;
        }
        void clear()
        {// virtual deallocation
            _virtAlloc(0);
            _virtualSize = 0;
        }
        void reset()
        {
            _dealloc();
            // this will give a default allocation
            _virtAlloc(1);
            _virtAlloc(0);
            _virtualSize = 0;
        }
        dword virtual_size() const { return _virtualSize; }
        dword actual_size() const { return _size(); }
    protected:
        typedef irregular_array_element<T> _Element;
    private:
        dword _virtualSize;
    };
}

#endif