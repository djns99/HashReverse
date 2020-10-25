#pragma once

namespace Math
{
    template<class T>
    T ceilDiv( T a,
               T b )
    {
        return (a + b - 1) / b;
    }
}