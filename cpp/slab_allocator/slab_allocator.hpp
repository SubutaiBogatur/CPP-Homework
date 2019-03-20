//
// Created by atukallo on 3/18/19.
//

#ifndef CSC_HW_SLAB_ALLOCATOR_HPP
#define CSC_HW_SLAB_ALLOCATOR_HPP

#include <limits>
#include "singleton_slab_allocator.hpp"

template<class T, size_t SlabSize>
struct slab_allocator {
    typedef T value_type;

    template <class U>
    struct rebind {
        using other = slab_allocator<U, SlabSize>;
    };

    slab_allocator() = default;

    template <class U, size_t SlabSizeOther>
    slab_allocator(const slab_allocator<U, SlabSizeOther> &) {
        // todo: seems wrong
    }

    T *allocate(size_t n);
    void deallocate(T *ptr, size_t n);

private:
    singleton_slab_allocator<SlabSize> &instance = singleton_slab_allocator<SlabSize>::get_instance();
};

template<class T, class U, size_t SlabSize1, size_t SlabSize2>
bool operator==(const slab_allocator<T, SlabSize1> &, const slab_allocator<U, SlabSize2> &) {
    return true; // if they share same singleton instance
}

template<class T, class U, size_t SlabSize1, size_t SlabSize2>
bool operator!=(const slab_allocator<T, SlabSize1> &a, const slab_allocator<U, SlabSize2> &b) {
    return false;
}

template<class T, size_t SlabSize>
T *slab_allocator<T, SlabSize>::allocate(size_t n) {
    return static_cast<T *>(instance.smalloc(n * sizeof(T)));
}

template<class T, size_t SlabSize>
void slab_allocator<T, SlabSize>::deallocate(T *ptr, size_t n) {
    instance.sfree(ptr);
}


#endif //CSC_HW_SLAB_HPP
