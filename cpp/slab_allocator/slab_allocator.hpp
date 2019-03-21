//
// Created by atukallo on 3/18/19.
//

#ifndef CSC_HW_SLAB_ALLOCATOR_HPP
#define CSC_HW_SLAB_ALLOCATOR_HPP

#include "singleton_slab_allocator.hpp"

namespace allocators {
    template<class T, size_t SlabSize = MIN_SLAB_SIZE>
    struct slab {
        typedef T value_type;

        template<class U>
        struct rebind {
            using other = slab<U, SlabSize>;
        };

        slab() = default;

        template<class U, size_t SlabSizeOther>
        explicit slab(const slab<U, SlabSizeOther> &) {
            // todo: seems wrong
        }

        T *allocate(size_t n);
        void deallocate(T *ptr, size_t n);

    private:
        singleton_slab_allocator<SlabSize> &instance = singleton_slab_allocator<SlabSize>::get_instance();
    };

    template<class T, class U, size_t SlabSize1, size_t SlabSize2>
    bool operator==(const slab<T, SlabSize1> &, const slab<U, SlabSize2> &) {
        // todo: check
        return true; // if they share same singleton instance
    }

    template<class T, class U, size_t SlabSize1, size_t SlabSize2>
    bool operator!=(const slab<T, SlabSize1> &a, const slab<U, SlabSize2> &b) {
        return !(a == b);
    }

    template<class T, size_t SlabSize>
    T *slab<T, SlabSize>::allocate(size_t n) {
        return static_cast<T *>(instance.smalloc(n * sizeof(T)));
    }

    template<class T, size_t SlabSize>
    void slab<T, SlabSize>::deallocate(T *ptr, size_t n) {
        instance.sfree(ptr, n);
    }

}


#endif //CSC_HW_SLAB_HPP
