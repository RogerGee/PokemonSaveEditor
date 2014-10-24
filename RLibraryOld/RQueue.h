//RQueue.h
#ifndef RQUEUE_H
#define RQUEUE_H
#include "RAllocator.h" // gets RTypesTypes.h

namespace rtypes
{
    template<class T>
    class queue : protected rallocator<T>
    {
    public:
        queue()
        {
            _head = 0;
            _tail = 0;
        }
        void push_back(const T& elem)
        {
            if (_tail>=_allocationSize())
            {
                _alloc()[_tail++] = elem;
                return;
            }
            _getData()[_tail++] = elem;
        }
        T pop_back()
        {
            if (is_empty())
                return T();
            if (_head+1==_tail)
            {// the queue is about to become empty
                T* elem = _getData()+_head;
                clear(); // promote efficient data usage
                return *elem;
            }
            return _getData()[_head++];
        }
        void pop_range(dword count)
        {// a "virtual" pop operation for "seeking" through the data
            _head += count;
            if (_head>_tail)
                clear();
        }
        const T& peek() const // it's okay to take the address of whatever this returns to look ahead in the queue, as long as it's not used after a subsequent push
        {
            static T badElem = T();
            if (is_empty())
                return badElem;
            return _getData()[_head];
        }
        T& peek() // non-const version
        {
            static T badElem = T();
            if (is_empty())
                return badElem;
            return _getData()[_head];
        }
        bool is_empty() const
        {
            return _head==_tail;
        }
        void clear() // doesn't reduce capacity
        {
            _head = 0;
            _tail = 0;
        }
        void reset() // reduces capacity
        {
            _head = 0;
            _tail = 0;
            _dealloc();
        }
        dword size() const
        {
            return _tail-_head;
        }
        dword capacity() const
        {
            return _allocationSize();
        }
    protected:
        using rallocator<T>::_allocationSize;
        using rallocator<T>::_getData;
        using rallocator<T>::_dealloc;
        using rallocator<T>::_alloc;
        virtual void _copy(T* copyTo,const T* copyFrom)
        {
            for (dword h = _head,i = 0;h<_tail;h++,i++)
                copyTo[i] = copyFrom[h];
        }
    private:
        dword _head, _tail;
    };

    template<class T>
    class wrapped_queue : protected rallocator<T>
    {
    public:
        wrapped_queue()
        {
            _head = 0;
            _tail = 0;
        }
        void push_back(const T& elem)
        {
            T* data;
            // check for ability to wrap
            if (_tail>=_allocationSize() && _head>1)
                _tail = 0; // wrap to unused indeces
            // check allocation and assign data
            if (_tail>=_allocationSize() || _tail==_head-1)
                data = _alloc(); // need more space
            else
                data = _getData();
            // push element
            data[_tail++] = elem;
        }
        T pop_back()
        {
            if (is_empty())
                return T();
            T* data = _getData();
            T& r = data[_head++];
            // check tail for wrapping
            if (_head>=_allocationSize() /* possible wrap */
                && _tail<_head /* is wrapping */)
                _head = 0;
            else if (is_empty())
                clear(); // use old space
            return r;
        }
        T peek() const
        {
            if (is_empty())
                return T();
            T* data = _getData();
            return data[_head];
        }
        bool is_empty() const { return _head==_tail; }
        void clear()
        {// maintain current capacity
            _head = 0;
            _tail = 0;
        }
        void reset()
        {
            _dealloc(); // reduce capacity to zero
            _alloc();
        }
        dword size() const
        {
            if (_tail<_head)
                return _allocationSize()-_head + _tail;
            return _tail-_head;
        }
        dword capacity() const // number of possible elements that could fit in queue
        {
            if (_head>1) // can wrap
                //      wrapped + linear = alloc size -1
                return _allocationSize()-1;
            return _allocationSize()-_head;
        }
    protected:
        using rallocator<T>::_allocationSize;
        using rallocator<T>::_getData;
        using rallocator<T>::_dealloc;
        using rallocator<T>::_alloc;
        virtual void _copy(T* copyTo,const T* copyFrom)
        {// callback for whenever a reallocation has occurred
            dword i = 0;
            if (_tail>_head)
            {// copy from head to tail
                for (;_head<_tail;_head++,i++)
                    copyTo[i] = copyFrom[_head];
            }
            else if (_tail<_head)
            {// copy from head to allocation size-1 and from 0 to _tail-1
                for (;_head<_allocationSize();_head++,i++)
                    copyTo[i] = copyFrom[_head];
                for (dword t = 0;t<_tail;t++,i++)
                    copyTo[i] = copyFrom[t];
            }
            // else is empty
            _head = 0;
            _tail = i;
        }
    private:
        dword _head, _tail;
    };

    template<class T,class P>
    struct priority_queue_elem
    {
        queue<T> data;
        P priority;
    };

    template<class T,class P>
    class priority_queue : protected rallocatorEx< priority_queue_elem<T,P>* >
    {
    public:
        priority_queue()
        {
        }
        priority_queue(const priority_queue& pq)
        {
            // define a custom copy operation since the
            // allocator's copy c-str will only copy the pointer values
            _virtAlloc(pq._size());
            _pq_data_ptr* thisData = _getData();
            _pq_data_ptr* thatData = pq._getData();
            for (dword i = 0;i<_size();i++)
            {
                thisData[i] = new priority_queue_elem<T,P>;
                *(thisData[i]) = *(thatData[i]);
            }
        }
        ~priority_queue()
        {
            _pq_data_ptr* data = _getData();
            for (dword i = 0;i<_size();i++)
                delete data[i];
        }
        priority_queue& operator =(const priority_queue& pq)
        {
            // define a custom copy operation since the allocator 
            // will only copy pointer values
            if (&pq==this)
                return *this;
            _virtAlloc(pq._size());
            _pq_data_ptr* thisData = _getData();
            _pq_data_ptr* thatData = pq._getData();
            for (dword i = 0;i<_size();i++)
                *(thisData[i]) = *(thatData[i]);
            return *this;
        }
        void push_back(const T& elem,const P& priority)
        {
            dword i = _searchPriority(priority);
            if (i<_size())
            {// use existing queue with priority
                _pq_data_ptr* data = _getData();
                data[i]->data.push_back(elem);
            }
            else
            {// create a new queue for the new priority
                _pq_data_ptr* data;
                _pq_data_ptr element = new priority_queue_elem<T,P>;
                element->priority = priority;
                element->data.push_back(elem);
                _virtAlloc(_size()+1);
                data = _getData();
                data[_size()-1] = element;
                _sortElems(); // sort for easy searching
            }
        }
        T pop_back()
        {
            // the elements are sorted, so the queue at the top bound has priority
            _pq_data_ptr* data = _getData();
            dword i = _size()-1;
            while (i<_size() && data[i]->data.is_empty())
                i--;
            if (i>=_size())
                return T(); // no element to pop (i wrapped to the top value)
            return data[i]->data.pop_back();
        }
        dword size() const
        {
            _pq_data_ptr* data = _getData();
            dword sz = 0;
            for (dword i = 0;i<_size();i++)
                sz += data[i]->data.size();
            return sz;
        }
        dword capacity() const
        {
            dword sum = 0;
            _pq_data_ptr* data = _getData();
            for (dword i = 0;i<_size();i++)
                sum += data[i]->data.capacity();
            return sum;
        }
        bool is_empty() const { return size()==0; }
        void clear() // maintains the current capacity (the number of priorities available to the pqueue)
        {
            _pq_data_ptr* data = _getData();
            for (dword i = 0;i<_size();i++)
                data[i]->data.clear();
        }
        void reset() // decreases capacity
        {
            _dealloc();
        }
    protected:
        typedef priority_queue_elem<T,P>* _pq_data_ptr;

        using rallocatorEx<_pq_data_ptr>::_allocationSize;
        using rallocatorEx<_pq_data_ptr>::_size;
        using rallocatorEx<_pq_data_ptr>::_getData;
        using rallocatorEx<_pq_data_ptr>::_dealloc;
        using rallocatorEx<_pq_data_ptr>::_copy;

    private:
        void _sortElems()
        {
            _pq_data_ptr* data = _getData();
            for (dword i = 1;i<_size();i++)
            {
                _pq_data_ptr pElem = data[i];
                P compVal = data[i]->priority;
                dword ind = i;
                while (ind>0 && compVal<data[ind-1]->priority)
                {
                    data[ind] = data[ind-1];
                    ind--;
                }
                if (ind!=i)
                    data[ind] = pElem;
            }
        }
        dword _searchPriority(const P& p)
        {
            _pq_data_ptr* data = _getData();
            int start = 0, end = _size()-1;
            while (start<=end)
            {
                int average = (start+end)/2;
                if (data[average]->priority==p)
                    return (dword) average;
                if (data[average]->priority<p)
                    start = average+1;
                else
                    end = average-1;
            }
            return _size();
        }
    };

}

#endif