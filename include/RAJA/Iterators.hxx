#ifndef RAJA_ITERATORS_HXX
#define RAJA_ITERATORS_HXX

#include "RAJA/int_datatypes.hxx"

#include <type_traits>
#include <iterator>
#include <utility>

namespace RAJA {
    namespace Iterators {


// Helpers


template<typename Container>
using IteratorCategoryOf = typename std::iterator_traits<typename Container::iterator>::iterator_category;

template<typename Container>
using OffersRAI =
    std::is_base_of<
        std::random_access_iterator_tag,
        IteratorCategoryOf<Container>>;

// Containers

template<typename Type,
         typename DifferenceType = std::ptrdiff_t,
         typename PointerType = Type *>
class base_iterator : public std::iterator<std::random_access_iterator_tag,
                                                Type,
                                                DifferenceType>
{
public:
    using difference_type = typename std::iterator<std::random_access_iterator_tag, Type>::difference_type;

    constexpr base_iterator() : val(0) {}
    constexpr base_iterator(Type rhs) : val(rhs) {}
    constexpr base_iterator(const base_iterator &rhs) : val(rhs.val) {}

    inline bool operator==(const base_iterator& rhs) const {return val == rhs.val;}
    inline bool operator!=(const base_iterator& rhs) const {return val != rhs.val;}
    inline bool operator>(const base_iterator& rhs) const {return val > rhs.val;}
    inline bool operator<(const base_iterator& rhs) const {return val < rhs.val;}
    inline bool operator>=(const base_iterator& rhs) const {return val >= rhs.val;}
    inline bool operator<=(const base_iterator& rhs) const {return val <= rhs.val;}
protected:
    Type val;
};

template<typename Type = Index_type,
         typename DifferenceType = Index_type,
         typename PointerType = Type *>
class numeric_iterator : public base_iterator< Type,
                                               DifferenceType>
{
public:
    using difference_type = typename std::iterator<std::random_access_iterator_tag, Type>::difference_type;
    using base = base_iterator<Type, DifferenceType>;

    constexpr numeric_iterator() : base(0) {}
    constexpr numeric_iterator(const Type& rhs) : base(rhs) {}
    constexpr numeric_iterator(const numeric_iterator& rhs) : base(rhs.val) {}

    inline numeric_iterator& operator++() {++base::val; return *this;}
    inline numeric_iterator& operator--() {--base::val; return *this;}
    inline numeric_iterator operator++(int) {numeric_iterator tmp(*this); ++base::val; return tmp;}
    inline numeric_iterator operator--(int) {numeric_iterator tmp(*this); --base::val; return tmp;}

    inline numeric_iterator& operator+=(const difference_type& rhs) {base::val+=rhs; return *this;}
    inline numeric_iterator& operator-=(const difference_type& rhs) {base::val-=rhs; return *this;}
    inline numeric_iterator& operator+=(const numeric_iterator& rhs) {base::val+=rhs.val; return *this;}
    inline numeric_iterator& operator-=(const numeric_iterator& rhs) {base::val-=rhs.val; return *this;}

    inline difference_type operator+(const numeric_iterator& rhs) const {return static_cast<difference_type>(base::val)+static_cast<difference_type>(rhs.val);}
    inline difference_type operator-(const numeric_iterator& rhs) const {return static_cast<difference_type>(base::val)-static_cast<difference_type>(rhs.val);}
    inline numeric_iterator operator+(const difference_type& rhs) const {return numeric_iterator(base::val+rhs);}
    inline numeric_iterator operator-(const difference_type& rhs) const {return numeric_iterator(base::val-rhs);}
    friend constexpr numeric_iterator operator+(difference_type lhs, const numeric_iterator& rhs) {return numeric_iterator(lhs+rhs.val);}
    friend constexpr numeric_iterator operator-(difference_type lhs, const numeric_iterator& rhs) {return numeric_iterator(lhs-rhs.val);}

    inline Type operator*() const {return base::val;}
    inline Type operator->() const {return base::val;}
    constexpr Type operator[](difference_type rhs) const {return base::val + rhs;}

};

template<typename Type = Index_type,
         typename DifferenceType = Index_type,
         typename PointerType = Type *>
class strided_numeric_iterator : public base_iterator< Type,
                                               DifferenceType>
{
public:
    using difference_type = typename std::iterator<std::random_access_iterator_tag, Type>::difference_type;
    using base = base_iterator<Type, DifferenceType>;

    constexpr strided_numeric_iterator() : base(0), stride(1) {}
    constexpr strided_numeric_iterator(const Type& rhs, DifferenceType stride = 1) : base(rhs), stride(stride) {}
    constexpr strided_numeric_iterator(const strided_numeric_iterator& rhs) : base(rhs.val), stride(rhs.stride) {}

    inline strided_numeric_iterator& operator++() {base::val += stride; return *this;}
    inline strided_numeric_iterator& operator--() {base::val -= stride; return *this;}

    inline strided_numeric_iterator& operator+=(const difference_type& rhs) {base::val+=rhs * stride; return *this;}
    inline strided_numeric_iterator& operator-=(const difference_type& rhs) {base::val-=rhs * stride; return *this;}

    inline difference_type operator+(const strided_numeric_iterator& rhs) const {
        return static_cast<difference_type>(base::val)
            + (static_cast<difference_type>(rhs.val * stride));}
    inline difference_type operator-(const strided_numeric_iterator& rhs) const {
        auto diff = static_cast<difference_type>(base::val)
            - (static_cast<difference_type>(rhs.val));
        if (diff < stride)
            return 0;
        if (diff % stride) // check for off-stride endpoint
            return diff/stride + 1;
        return diff/stride;
    }
    inline strided_numeric_iterator operator+(const difference_type& rhs) const {return strided_numeric_iterator(base::val+rhs * stride);}
    inline strided_numeric_iterator operator-(const difference_type& rhs) const {return strided_numeric_iterator(base::val-rhs * stride);}

    // Specialized comparison to allow normal iteration to work on off-stride
    // multiples by adjusting rhs to the nearest *higher* multiple of stride
    inline bool operator!=(const strided_numeric_iterator& rhs) const {
        if (base::val == rhs.val) return false;
        auto rem = rhs.val % stride;
        return base::val != rhs.val + rem;
    }

    inline Type operator*() const {return base::val;}
    inline Type operator->() const {return base::val;}
    inline Type operator[](difference_type rhs) const {return base::val + rhs * stride;}

private:
    DifferenceType stride;

};

// TODO: this should really be a generic Zip, then using Enumerator =
// Zip<numeric_iterator, Iterator>
template<typename Iterator>
class Enumerater : public numeric_iterator<> {
public:
    using base = numeric_iterator<>;

    using pair = std::pair<std::ptrdiff_t,
                           Iterator>;
    using value_type = pair;
    using pointer_type = pair*;
    using reference = pair&;

    Enumerater() = delete;
    constexpr Enumerater(const Iterator& rhs,
                         std::ptrdiff_t val = 0,
                         std::ptrdiff_t offset = 0)
        : base(val), offset(offset), wrapped(rhs) {}
    constexpr Enumerater(const Enumerater &rhs)
        : base(rhs.val), offset(rhs.offset), wrapped(rhs.wrapped) {}

    inline pair operator*() const {return pair(offset+val, wrapped+val);}
    constexpr pair operator[](typename base::difference_type rhs) const {
        return pair(val+offset+rhs, wrapped+val+rhs);
    }

private:
    std::ptrdiff_t offset;
    Iterator wrapped;
};

}
}

#endif /* RAJA_ITERATORS_HXX */
