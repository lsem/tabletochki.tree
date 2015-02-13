#ifndef __UTILS_H_INCLUDED
#define __UTILS_H_INCLUDED

namespace Utils 
{

    /*
    *  This Quickselect routine is based on the algorithm described in
    *  "Numerical recipes in C", Second Edition,
    *  Cambridge University Press, 1992, Section 8.5, ISBN 0-521-43108-5
    *  This code by Nicolas Devillard - 1998. Public domain.
    */


#define ELEM_SWAP(a,b) { register T t=(a);(a)=(b);(b)=t; }

    template <class T>
    T QuickSelect(T arr[], int n)
    {
        int low, high;
        int median;
        int middle, ll, hh;

        low = 0; high = n - 1; median = (low + high) / 2;
        for (;;) {
            if (high <= low) /* One element only */
                return arr[median];

            if (high == low + 1) {  /* Two elements only */
                if (arr[low] > arr[high])
                    ELEM_SWAP(arr[low], arr[high]);
                return arr[median];
            }

            /* Find median of low, middle and high items; swap into position low */
            middle = (low + high) / 2;
            if (arr[middle] > arr[high])    ELEM_SWAP(arr[middle], arr[high]);
            if (arr[low] > arr[high])       ELEM_SWAP(arr[low], arr[high]);
            if (arr[middle] > arr[low])     ELEM_SWAP(arr[middle], arr[low]);

            /* Swap low item (now in position middle) into position (low+1) */
            ELEM_SWAP(arr[middle], arr[low + 1]);

            /* Nibble from each end towards middle, swapping items when stuck */
            ll = low + 1;
            hh = high;
            for (;;) {
                do ll++; while (arr[low] > arr[ll]);
                do hh--; while (arr[hh]  > arr[low]);

                if (hh < ll)
                    break;

                ELEM_SWAP(arr[ll], arr[hh]);
            }

            /* Swap middle item (in position low) back into correct position */
            ELEM_SWAP(arr[low], arr[hh]);

            /* Re-set active partition */
            if (hh <= median)
                low = ll;
            if (hh >= median)
                high = hh - 1;
        }
    }

#undef ELEM_SWAP

	inline uint16_t Fletcher16(const uint8_t* data, int count )
	{
	   uint16_t sum1 = 0;
	   uint16_t sum2 = 0;
	   int index;
	 
	   for( index = 0; index < count; ++index )
	   {
	      sum1 = (sum1 + data[index]) % 255;
	      sum2 = (sum2 + sum1) % 255;
	   }
	 
	   return (sum2 << 8) | sum1;
	}
    
    inline uint16_t MakeWord(uint8_t highByte, uint8_t lowByte)
    {
        return (uint16_t) highByte << 8 | lowByte;
    }

    inline uint16_t HostToNetworkUI16(uint16_t value)
    {
        return (uint16_t)(value << 8) | (uint16_t)(value >> 8);
    }


    template <class T>
    static inline bool InRange(T value, T min, T max)
    {
        return (value >= min) && (value < max);
    }

    template <class FromType, class ToType, FromType RangeBegin, FromType RangeEnd>
    class StaticMap
    {
    public:
        static const ToType GetMappedValue(FromType fromElement) { ASSERT(InRange(fromElement, RangeBegin, RangeEnd)); return SourceElements[fromElement]; }
        static const ToType *GetMappedElementsStoragePtr() { return SourceElements; }
        static const size_t GetMappedElementCount() { return RangeEnd - RangeBegin; }
    private:
        static const ToType SourceElements[RangeEnd - RangeBegin];
    };
}

//////////////////////////////////////////////////////////////////////////

#define STATIC_MAP(InstanceName, TypeFrom, TypeTo, RangeBegin, RangeEnd) \
    Utils::StaticMap<TypeFrom, TypeTo, RangeBegin, RangeEnd> InstanceName; \
    template <> const TypeTo Utils::StaticMap<TypeFrom, TypeTo, RangeBegin, RangeEnd> ::SourceElements[] =

#define STATIC_MAP_INSTANCE(InstanceName, TypeFrom, TypeTo, RangeBegin, RangeEnd) \
    Utils::StaticMap<TypeFrom, TypeTo, RangeBegin, RangeEnd> InstanceName;

#define CLASS_DECLARATION_FOR_FRIENDSHIP(InstanceName, TypeFrom, TypeTo, RangeBegin, RangeEnd) Utils::StaticMap<TypeFrom, TypeTo, RangeBegin, RangeEnd>


#endif // __UTILS_H_INCLUDED
