//RList.h
#ifndef RLIST_H
#define RLIST_H
#include "RTypesTypes.h"

#ifndef NULL
#define NULL 0
#endif

namespace rtypes
{
    template<class T>
    class list
    {
    public:
        list()
        {
            _head = NULL;
            _tail = NULL;
            _sz = 0;
        }
        list(const list& obj)
        {
            _head = NULL;
            _tail = NULL;
            _sz = 0;
            rnode* walker = obj._head;
            while (walker!=NULL)
            {
                add(walker->data);
                walker = walker->next;
            }
        }
        ~list()
        {
            _delete();
        }
        list& operator =(const list& obj)
        {
            if (&obj==this)
                return *this;
            rnode* walker = obj._head;
            empty();
            while (walker!=NULL)
            {
                add(walker->data);
                walker = walker->next;
            }
            return *this;
        }
        T& operator [](dword atIndex) { return get_elem(atIndex); }
        const T& operator [](dword atIndex) const { return get_elem(atIndex); }
        void add()
        {
            rnode* newNode = new rnode;
            _sz++;
            if (_head==NULL)
            {
                _head = newNode;
                _tail = newNode;
                return;
            }
            _tail->next = newNode;
            newNode->prev = _tail;
            _tail = newNode;
        }
        void add(const T& elem)
        {
            rnode* newNode = new rnode;
            newNode->data = elem;
            _sz++;
            if (_head==NULL)
            {
                _head = newNode;
                _tail = newNode;
                return;
            }
            _tail->next = newNode;
            newNode->prev = _tail;
            _tail = newNode;
        }
        bool insert(const T& elem,dword atIndex)
        {
            if (atIndex>_sz) // can still be equal to size (which adds)
                return false;
            rnode* insBefore = _head;
            dword index = 0;
            while (insBefore!=NULL)
            {
                if (index==atIndex)
                    break;
                insBefore = insBefore->next;
                index++;
            }
            rnode* newNode = new rnode;
            newNode->data = elem;
            if (insBefore==NULL)
            {
                if (_sz==0)
                    _head = newNode;
                else
                {
                    newNode->prev = _tail;
                    _tail->next = newNode;
                }
                _tail = newNode;
            }
            else
            {
                newNode->next = insBefore;
                newNode->prev = insBefore->prev;
                (insBefore->prev==NULL ? _head : insBefore->prev->next) = newNode;
                (insBefore->next==NULL ? _tail : insBefore->prev) = newNode;
            }
            _sz++;
            return true;
        }
        bool remove_at(dword atIndex)
        {
            rnode* del = _head;
            dword index = 0;
            while (del!=NULL)
            {
                if (index==atIndex)
                {
                    (del->prev==NULL ? _head : del->prev->next) = del->next;
                    (del->next==NULL ? _tail : del->next->prev) = del->prev;
                    _sz--;
                    delete del;
                    return true;
                }
                del = del->next;
                index++;
            }
            return false;
        }
        T& get_elem(dword atIndex)
        {
            static T badElem = T();
            dword index = 0;
            rnode* walker = _head;
            while (walker!=NULL)
            {
                if (index==atIndex)
                    return walker->data;
                walker = walker->next;
                index++;
            }
            return badElem;
        }
        const T& get_elem(dword atIndex) const
        {
            static T badElem = T();
            dword index = 0;
            rnode* walker = _head;
            while (walker!=NULL)
            {
                if (index==atIndex)
                    return walker->data;
                walker = walker->next;
                index++;
            }
            return badElem;
        }
        void empty()
        {
            _delete();
        }
        bool is_empty() const
        {
            return _sz==0;
        }
        dword size() const
        {
            return _sz;
        }
        dword length() const
        {
            return _sz;
        }
        void quicksort()
        {
            _quicksortRecursive(_head,_tail,_sz);
        }
    protected:
        struct rnode
        {
            rnode() : prev(NULL),next(NULL) {}
            rnode* prev;
            rnode* next;
            T data;
        };
    private:
        static void _quicksortRecursive(rnode* &head,rnode* &tail,dword size)
        {
            if (head==NULL || size==0)
                return;
            rnode* walker = head, *equal = NULL, *less = NULL, *greater = NULL, *dummy = NULL;
            dword eqSize = 0, lessSize = 0, greaterSize = 0;
            list<T> list;
            list._head = head;
            list._sz = size;
            T pivot = list.get_elem(size/2);
            while (walker!=NULL)
            {
                rnode* next = walker->next;
                walker->prev = NULL;
                if (walker->data==pivot)
                {
                    walker->next = equal;
                    if (equal!=NULL)
                        equal->prev = walker;
                    equal = walker;
                    eqSize++;
                }
                else if (walker->data<pivot)
                {
                    walker->next = less;
                    if (less!=NULL)
                        less->prev = walker;
                    less = walker;
                    lessSize++;
                }
                else
                {
                    walker->next = greater;
                    if (greater!=NULL)
                        greater->prev = walker;
                    greater = walker;
                    greaterSize++;
                }
                walker = next;
            }
            _quicksortRecursive(less,dummy,lessSize);
            _quicksortRecursive(greater,dummy,greaterSize);
            if (less!=NULL)
            {
                rnode* temp;
                list._head = less;
                list._sz = lessSize;
                temp = list._getNode(lessSize-1);
                temp->next = equal;
                equal->prev = temp;
                head = less;
            }
            else
                head = equal;
            list._head = equal;
            list._sz = eqSize;
            walker = list._getNode(eqSize-1);
            tail = walker;
            if (greater!=NULL)
            {
                walker->next = greater;
                greater->prev = walker;
                list._head = greater;
                list._sz = greaterSize;
                tail = list._getNode(greaterSize-1);
            }
            // stop the destructor from working on the list object
            list._head = NULL;
            list._sz = 0;
        }

        rnode* _head;
        rnode* _tail;
        dword _sz;

        void _delete()
        {
            rnode* walker = _head;
            while (walker!=NULL)
            {
                rnode* tmp = walker->next;
                delete walker;
                walker = tmp;
            }
            _head = NULL;
            _tail = NULL;
            _sz = 0;
        }
        rnode* _getNode(dword Index)
        {
            rnode* r = _head;
            for (dword i = 0;i<Index && r!=NULL;i++, r = r->next);
            return r;
        }
    };
}

#endif