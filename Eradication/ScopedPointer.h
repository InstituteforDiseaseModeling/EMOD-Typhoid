/***************************************************************************************************

Copyright (c) 2015 Intellectual Ventures Property Holdings, LLC (IVPH) All rights reserved.

EMOD is licensed under the Creative Commons Attribution-Noncommercial-ShareAlike 4.0 License.
To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/legalcode

***************************************************************************************************/

#pragma once

// Patterned after (simplified from) b005t::scoped-ptr

// Guarantees deletion of the object pointed to on destruction of the scoped_pointer.

namespace idm
{
    template<class T> class scoped_ptr // noncopyable
    {
    public:

        typedef T element_type;

        explicit scoped_ptr( T* p = nullptr ) : px( p ) {}  // never throws

        ~scoped_ptr() // never throws
        {
            delete px;
        }

        T& operator*() const // never throws
        {
            return *px;
        }

        T* operator->() const // never throws
        {
            return px;
        }

    private:

        T * px;

        scoped_ptr(scoped_ptr const &);
        scoped_ptr & operator=(scoped_ptr const &);

        typedef scoped_ptr<T> this_type;

        void operator==( scoped_ptr const& ) const;
        void operator!=( scoped_ptr const& ) const;
    };
}
