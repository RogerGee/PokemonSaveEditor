//RTypes.h - generic types for RLibrary
#ifndef RARRAY_H
#define RARRAY_H

namespace rtypes
{
    template<class T,const int SIZE>
    class fixed_array
    {
    public:
        //fixed_array()
        //{
        //}
        const T& get_elem(unsigned int at) const
        {
            return _data[at];
        }
        T& get_elem(unsigned int at)
        {
            return _data[at];
        }
        const T& operator [](unsigned int at) const
        {
            return _data[at];
        }
        T& operator [](unsigned int at)
        {
            return _data[at];
        }
        int size() const { return SIZE; }
    private:
        T _data[SIZE];
    };

    template<class T>
    class sized_container
    {
    public:
        sized_container()
        {
            _data = 0;
            _sz = 0;
        }
        sized_container(unsigned int Size)
        {
            _sz = Size;
            _data = 0;
            _alloc();
        }
        sized_container(const sized_container& Obj)
        {
            _sz = Obj._sz;
            _data = 0;
            _alloc();
            for (unsigned int i = 0;i<_sz;i++)
                _data[i] = Obj[i];
        }
        ~sized_container()
        {
            _sz = 0;
            _alloc(); // alloc only reallocs if _sz>0
        }
        void resize(unsigned int Size)
        {
            _sz = Size;
            _alloc();
        }
        void insertion_sort()
        {
            for (unsigned int i = 1;i<_sz;i++)
            {
                T elem = _data[i];
                unsigned int ind = i;
                while (ind>0 && elem<_data[ind-1])
                {
                    _data[ind] = _data[ind-1];
                    ind--;
                }
                if (ind!=i)
                    _data[ind] = elem;
            }
        }
        const T& operator [](unsigned int Ind) const
        {
            return _data[Ind];
        }
        T& operator [](unsigned int Ind)
        {
            return _data[Ind];
        }
        sized_container& operator =(const sized_container& Obj)
        {
            _sz = Obj._sz;
            _alloc();
            for (unsigned int i = 0;i<_sz;i++)
                _data[i] = Obj[i];
            return *this;
        }
        sized_container& operator =(const T Array[])
        {
            // I'll assume the array is the right size
            for (unsigned int i = 0;i<_sz;i++)
                _data[i] = Array[i];
            return *this;
        }
        unsigned int size() const { return _sz; }
    private:
        T* _data;
        unsigned int _sz;
        void _alloc()
        {
            delete[] _data;
            _data = 0;
            if (_sz)
                _data = new T[_sz];
        }
    };
}

#endif