//
// Created by atukallo on 3/16/19.
//

#ifndef CSC_HW_SINGLTON_SLAB_ALLOCATOR_HPP
#define CSC_HW_SINGLTON_SLAB_ALLOCATOR_HPP

#include <stddef.h> // for size_t
#include <cstdint> // for uint8_t
#include <cassert> // for assert
#include <stdlib.h> // for aligned_alloc
#include <optional>
#include <array>

#include <iostream>
#include "easy_logging.hpp"

static const size_t MIN_SLAB_SIZE = 4096;
static const size_t SLAB_TYPES = 8; // slabs of pows from 2^3 to 2^(2+SLAB_TYPES) inclusively
static const size_t MAX_CELL_SIZE = 1 << (2 + SLAB_TYPES);
static const size_t RESERVED_BYTES = 64;

using namespace logging;

template<size_t SlabSize = MIN_SLAB_SIZE>
struct singleton_slab_allocator {
    static_assert(SlabSize >= MIN_SLAB_SIZE && (SlabSize & (SlabSize - 1)) == 0); // check if is big & is power of two

    static singleton_slab_allocator &get_instance() {
        static singleton_slab_allocator instance;
        return instance;
    }

    void *smalloc(size_t n); // takes O(1), n -- number of bytes
    void sfree(void *ptr, size_t n); // takes O(1)
    void clear(); // takes O(|empty slabs|), returns all the empty slabs memory to os

    size_t allocated_slabs(size_t cell_size) const; // takes O(|allocated_slabs|)
    size_t capacity(size_t cell_size) const; // takes O(|allocated_slabs|)
    size_t size(size_t cell_size) const; // returns allocated_cells_num, takes O(|allocated_slabs|)

private:
    singleton_slab_allocator();

    // allocator has slabs with cells of different sizes: 8, 16, 32, ..., 1024 - 8 different slab types
    // all the free cells are connected in free-list
    // every slab has size SlabSize, first 64 bytes of slab are reserved for following data in the following order:
    // size_t cell_size, size_t free_cells_cnt, void *free_list_head, void *prev_slab, void *next_slab -- totally 40 bytes, getters and setters are provided
    // though it's not obvious, 8 bytes for size_t are reserved (it's size is implementation dependent)
    // for every cell size heads for 3 lists are stored: empty_slabs_list, filled_slabs_list, wip_slabs_list
    // after (de)?allocation slabs may be moved from one list to another
    // all slabs are aligned by SlabSize

    // to get from cell size index, do: log_2(x) - 3. In slab start saved in this form
    std::array<void *, SLAB_TYPES> empty_slabs_lists;
    std::array<void *, SLAB_TYPES> wip_slabs_lists; // work-in-progress
    std::array<void *, SLAB_TYPES> filled_slabs_lists;

    static void *new_slab(size_t cell_size); // aligned allocation & free-list construction

    static bool is_slab_empty(void *slab);
    static bool is_slab_filled(void *slab);

    // helpers for list operations, slab|cell!=nullptr, head mb nullptr:
    static void *remove_from_slab_list(void *head_slab, void *slab); // either new head or head_slab is returned
    static void *push_to_slab_list(void *head_slab, void *slab); // head is returned
    static void push_to_cell_list(void *slab, void *cell); // modifies reserved area & cell
    static void *pop_from_cell_list(void *slab); // modifies reserved area

    // helpers for reserved area, slab != nullptr:
    static size_t get_cell_size(void *slab);
    static void set_cell_size(void *slab, size_t val);
    static size_t get_free_cells_cnt(void *slab);
    static void set_free_cells_cnt(void *slab, size_t val);
    static void *get_free_list_head(void *slab);
    static void set_free_list_head(void *slab, void *val);
    static void *get_prev_slab(void *slab);
    static void set_prev_slab(void *slab, void *val);
    static void *get_next_slab(void *slab);
    static void set_next_slab(void *slab, void *val);
    static void set_next_prev_slab(void *slab, void *val);
    static void *get_next_from_cell(void *cell);
    static void set_next_to_cell(void *cell, void *val);

    static size_t get_max_cells_in_slab(size_t cell_size);
    static size_t cell_size_from_index(size_t i);
    static size_t index_from_cell_size(size_t cell_size);
    static void *get_slab_from_cell(void *cell);

    // utils:
    static size_t powui(size_t base, size_t pow);
    static size_t ceiling_pow2(size_t val); // find pow2 >= val
};

template<size_t SlabSize>
singleton_slab_allocator<SlabSize>::singleton_slab_allocator() {
    for (size_t i = 0; i < SLAB_TYPES; i++) {
        empty_slabs_lists[i] = nullptr;
        wip_slabs_lists[i] = nullptr;
        filled_slabs_lists[i] = nullptr;

        void *empty_slab = new_slab(cell_size_from_index(i));
        empty_slabs_lists[i] = push_to_slab_list(empty_slabs_lists[i], empty_slab);
    }
    info() << "allocated initial slabs";
}

template<size_t SlabSize>
void *singleton_slab_allocator<SlabSize>::new_slab(size_t cell_size) {
    void *slab = aligned_alloc(SlabSize, SlabSize);
    size_t cells_cnt = get_max_cells_in_slab(cell_size); // at the end mb another empty zone, who cares

    info() << "making new slab for cell_size: " << cell_size << " with " << cells_cnt << " cells";

    set_cell_size(slab, cell_size);
    set_free_cells_cnt(slab, cells_cnt);
    set_next_prev_slab(slab, nullptr);

    // build free-list
    void *free_list_head = static_cast<char *>(slab) + RESERVED_BYTES;
    for (size_t i = 0; i < cells_cnt; i++) {
        void *cell_start = static_cast<char *>(free_list_head) + i * cell_size;
        if (i == cells_cnt - 1) {
            set_next_to_cell(cell_start, nullptr); // last one
        } else {
            void *next_cell_start = static_cast<char *>(cell_start) + cell_size;
            set_next_to_cell(cell_start, next_cell_start);
        }
    }

    set_free_list_head(slab, free_list_head);

    return slab;
}


template<size_t SlabSize>
void *singleton_slab_allocator<SlabSize>::smalloc(size_t n) {
    info() << "doing smalloc for n: " << n;

    if (n > MAX_CELL_SIZE) {
        return malloc(n); // alignment probably not needed
    }

    size_t cell_size = ceiling_pow2(n);
    size_t i = index_from_cell_size(cell_size);

    if (wip_slabs_lists[i] == nullptr && empty_slabs_lists[i] == nullptr) {
        void *empty_slab = new_slab(cell_size);
        empty_slabs_lists[i] = push_to_slab_list(empty_slabs_lists[i], empty_slab);
    }

    if (wip_slabs_lists[i] == nullptr && empty_slabs_lists[i] != nullptr) {
        // todo: should corrupt smth jff later
        void *empty_slab_list_head = empty_slabs_lists[i];
        empty_slabs_lists[i] = remove_from_slab_list(empty_slab_list_head, empty_slab_list_head); // pop head
        wip_slabs_lists[i] = push_to_slab_list(wip_slabs_lists[i], empty_slab_list_head);
    }

    void *wip_slab = wip_slabs_lists[i];
    void *cell = pop_from_cell_list(wip_slab);
    if (is_slab_filled(wip_slab)) {
        wip_slabs_lists[i] = remove_from_slab_list(wip_slab, wip_slab);
        filled_slabs_lists[i] = push_to_slab_list(filled_slabs_lists[i], wip_slab);
    }

    debug() << "allocated " << get_max_cells_in_slab(cell_size) - get_free_cells_cnt(wip_slab) << "th cell in slab for size " << cell_size;

    return cell;
}

template<size_t SlabSize>
void singleton_slab_allocator<SlabSize>::sfree(void *ptr, size_t n) {
    info() << "doing sfree";

    if (n > MAX_CELL_SIZE) {
        free(ptr); // can possible store search tree (or even Van Emde Boas tree!!) on ptrs and then not use size
        return;
    }

    void *slab = get_slab_from_cell(ptr);
    push_to_cell_list(slab, ptr);

    size_t cell_size = get_cell_size(slab);
    size_t i = index_from_cell_size(cell_size);

    if (get_free_cells_cnt(slab) == 1) { // became wip
        filled_slabs_lists[i] = remove_from_slab_list(filled_slabs_lists[i], slab);
        wip_slabs_lists[i] = push_to_slab_list(wip_slabs_lists[i], slab);
    } else if (is_slab_empty(slab)) { // became empty
        wip_slabs_lists[i] = remove_from_slab_list(wip_slabs_lists[i], slab);
        empty_slabs_lists[i] = push_to_slab_list(empty_slabs_lists[i], slab);
    }
}

template<size_t SlabSize>
void singleton_slab_allocator<SlabSize>::clear() {
    // free all unused slabs
    for (size_t i = 0; i < SLAB_TYPES; i++) {
        while (empty_slabs_lists[i] != nullptr) {
            void *head = empty_slabs_lists[i];
            empty_slabs_lists[i] = remove_from_slab_list(head, head);
            free(head);
        }
    }
}

template<size_t SlabSize>
bool singleton_slab_allocator<SlabSize>::is_slab_empty(void *slab) {
    return get_free_cells_cnt(slab) == get_max_cells_in_slab(get_cell_size(slab));
}

template<size_t SlabSize>
bool singleton_slab_allocator<SlabSize>::is_slab_filled(void *slab) {
    return get_free_cells_cnt(slab) == 0;
}

template<size_t SlabSize>
void *singleton_slab_allocator<SlabSize>::remove_from_slab_list(void *head_slab, void *slab) {
    void *prev_slab = get_prev_slab(slab);
    void *next_slab = get_next_slab(slab);

    if (prev_slab == nullptr && next_slab == nullptr) {
        return nullptr; // was only element in list & list became empty
    }

    if (prev_slab == nullptr) {
        set_next_slab(slab, nullptr); // jff
        set_prev_slab(next_slab, nullptr);
        return next_slab;
    }

    if (next_slab == nullptr) {
        set_prev_slab(slab, nullptr);
        set_next_slab(prev_slab, nullptr);
        return head_slab;
    }

    set_next_slab(prev_slab, next_slab);
    set_prev_slab(next_slab, prev_slab);
    set_next_prev_slab(slab, nullptr);

    return head_slab;
}

template<size_t SlabSize>
void *singleton_slab_allocator<SlabSize>::push_to_slab_list(void *head_slab, void *slab) {
    // always added as a first one. Why?
    // order is important only in wip_list. When adding there?
    // from empty_list - when wip_list is empty => doesn't matter
    // from filled_list - when single cell deletion was made from filled_list
    // new cells are taken from head of wip_list, so inserting better be done in the beginning, so almost-filled elements
    // become filled once again sooner

    if (head_slab == nullptr) {
        set_next_prev_slab(slab, nullptr);
        return slab;
    }

    set_prev_slab(head_slab, slab);
    set_next_slab(slab, head_slab);

    return slab;
}

template<size_t SlabSize>
void singleton_slab_allocator<SlabSize>::push_to_cell_list(void *slab, void *cell) {
    assert(!is_slab_empty(slab));

    void *free_list_head = get_free_list_head(slab);
    if (free_list_head == nullptr) {
        set_next_to_cell(cell, nullptr);
    } else {
        set_next_to_cell(cell, free_list_head);
    }
    set_free_list_head(slab, cell);

    size_t free_cells_cnt = get_free_cells_cnt(slab);
    set_free_cells_cnt(slab, free_cells_cnt + 1);
}

template<size_t SlabSize>
void *singleton_slab_allocator<SlabSize>::pop_from_cell_list(void *slab) {
    assert(!is_slab_filled(slab));

    size_t free_cells_cnt = get_free_cells_cnt(slab);

    void *free_list_head = get_free_list_head(slab); // cannot be nullptr be invariants
    void *second_list_node = get_next_from_cell(free_list_head); // mb nullptr, np
    set_free_list_head(slab, second_list_node);
    set_free_cells_cnt(slab, free_cells_cnt - 1);

    return free_list_head;
}

template<size_t SlabSize>
size_t singleton_slab_allocator<SlabSize>::get_cell_size(void *slab) {
    void *shifted_slab = slab;
    return *static_cast<size_t *>(shifted_slab);
}

template<size_t SlabSize>
void singleton_slab_allocator<SlabSize>::set_cell_size(void *slab, size_t val) {
    void *shifted_slab = slab;
    *static_cast<size_t *>(shifted_slab) = val;
}

template<size_t SlabSize>
size_t singleton_slab_allocator<SlabSize>::get_free_cells_cnt(void *slab) {
    void *shifted_slab = static_cast<char *>(slab) + 8;
    return *static_cast<size_t *>(shifted_slab);
}

template<size_t SlabSize>
void singleton_slab_allocator<SlabSize>::set_free_cells_cnt(void *slab, size_t val) {
    void *shifted_slab = static_cast<char *>(slab) + 8;
    *static_cast<size_t *>(shifted_slab) = val;
}

template<size_t SlabSize>
void *singleton_slab_allocator<SlabSize>::get_free_list_head(void *slab) {
    void *shifted_slab = static_cast<char *>(slab) + 16;
    return *static_cast<void **>(shifted_slab);
}

template<size_t SlabSize>
void singleton_slab_allocator<SlabSize>::set_free_list_head(void *slab, void *val) {
    void *shifted_slab = static_cast<char *>(slab) + 16;
    *static_cast<void **>(shifted_slab) = val;
}

template<size_t SlabSize>
void *singleton_slab_allocator<SlabSize>::get_prev_slab(void *slab) {
    void *shifted_slab = static_cast<char *>(slab) + 24;
    return *static_cast<void **>(shifted_slab);
}

template<size_t SlabSize>
void singleton_slab_allocator<SlabSize>::set_prev_slab(void *slab, void *val) {
    void *shifted_slab = static_cast<char *>(slab) + 24;
    *static_cast<void **>(shifted_slab) = val;
}

template<size_t SlabSize>
void *singleton_slab_allocator<SlabSize>::get_next_slab(void *slab) {
    void *shifted_slab = static_cast<char *>(slab) + 32;
    return *static_cast<void **>(shifted_slab);
}

template<size_t SlabSize>
void singleton_slab_allocator<SlabSize>::set_next_slab(void *slab, void *val) {
    void *shifted_slab = static_cast<char *>(slab) + 32;
    *static_cast<void **>(shifted_slab) = val;
}

template<size_t SlabSize>
void singleton_slab_allocator<SlabSize>::set_next_prev_slab(void *slab, void *val) {
    set_next_slab(slab, val);
    set_prev_slab(slab, val);
}

template<size_t SlabSize>
void *singleton_slab_allocator<SlabSize>::get_next_from_cell(void *cell) {
    return *static_cast<void **>(cell);
}

template<size_t SlabSize>
void singleton_slab_allocator<SlabSize>::set_next_to_cell(void *cell, void *val) {
    *static_cast<void **>(cell) = val;
}

template<size_t SlabSize>
size_t singleton_slab_allocator<SlabSize>::powui(size_t base, size_t pow) {
    size_t ret = 1;
    for (size_t i = 0; i < pow; i++) {
        ret *= base;
    }
    return ret;
}

template<size_t SlabSize>
size_t singleton_slab_allocator<SlabSize>::ceiling_pow2(size_t val) {
    size_t v = val;

    v--;
    v |= v >> 1;
    v |= v >> 2;
    v |= v >> 4;
    v |= v >> 8;
    v |= v >> 16;
    v |= sizeof(size_t) > 32 ? v >> 32 : v; // mb on some architectures 64 bits...
    v++;

    return v;
}

template<size_t SlabSize>
size_t singleton_slab_allocator<SlabSize>::get_max_cells_in_slab(size_t cell_size) {
    return (SlabSize - RESERVED_BYTES) / cell_size;
}

template<size_t SlabSize>
size_t singleton_slab_allocator<SlabSize>::cell_size_from_index(size_t i) {
    return powui(2, i + 3);
}

template<size_t SlabSize>
size_t singleton_slab_allocator<SlabSize>::index_from_cell_size(size_t cell_size) {
    for (size_t i = 0; i < SLAB_TYPES; i++) {
        if (powui(2, i + 3) == cell_size) {
            return i;
        }
    }

    assert(false);
    return 666;
}

template<size_t SlabSize>
void *singleton_slab_allocator<SlabSize>::get_slab_from_cell(void *cell) {

    // todo: why static_cast doesn't work?
    uintptr_t cell_uint = (uintptr_t) cell;
    cell_uint = cell_uint - cell_uint % SlabSize;
    void *slab = (void *) cell_uint;

    return slab;
}

#endif //CSC_HW_SINGLTON_SLAB_ALLOCATOR_HPP
