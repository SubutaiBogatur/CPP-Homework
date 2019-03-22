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
        slab(slab const &other) = default; // just copy or move, shares instance as ref

        slab &operator=(slab const &) = delete; // don't need & don't want assignment

        template<class U, size_t SlabSizeOther>
        explicit slab(const slab<U, SlabSizeOther> &) {
            // explicit forbids using syntax A a = b; only A a(b);
            // we need A(b) == a, so don't need anything special
        }

        T *allocate(size_t n);
        void deallocate(T *ptr, size_t n);

        void clear();
        size_t allocated_slabs(size_t cell_size) const;
        size_t capacity(size_t cell_size) const;
        size_t size(size_t cell_size) const;

    private:
        singleton_slab_allocator<SlabSize> &instance = singleton_slab_allocator<SlabSize>::get_instance();
    };

    template<class T, class U, size_t SlabSize1, size_t SlabSize2>
    bool operator==(const slab<T, SlabSize1> &, const slab<U, SlabSize2> &) {
        return SlabSize1 == SlabSize2; // true, if allocated by one, can be freed by another
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

    template<class T, size_t SlabSize>
    void slab<T, SlabSize>::clear() {
        instance.clear();
    }

    template<class T, size_t SlabSize>
    size_t slab<T, SlabSize>::allocated_slabs(size_t cell_size) const {
        return instance.allocated_slabs(cell_size);
    }

    template<class T, size_t SlabSize>
    size_t slab<T, SlabSize>::capacity(size_t cell_size) const {
        return instance.capacity(cell_size);
    }

    template<class T, size_t SlabSize>
    size_t slab<T, SlabSize>::size(size_t cell_size) const {
        return instance.size(cell_size);
    }
}


#endif //CSC_HW_SLAB_HPP
