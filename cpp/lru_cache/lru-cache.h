#ifndef LRU_CACHE_H
#define LRU_CACHE_H

#include <string>
#include <cstdint>

typedef uint32_t key_type;
typedef std::string mapped_type;
typedef std::pair<key_type, mapped_type> value_type;

struct lru_cache
{
    struct iterator;

    // Создает пустой lru_cache с указанной capacity.
    lru_cache(size_t cap);

    // Деструктор. Вызывается при удалении объектов lru_cache.
    // Инвалидирует все итераторы ссылающиеся на элементы этого lru_cache
    // (включая итераторы ссылающиеся на элементы следующие за последними).
    ~lru_cache();

    // Поиск элемента.
    // Возвращает итератор на элемент найденный элемент, либо end().
    // Если элемент найден, он помечается как наиболее поздно использованный.
    iterator find(key_type);

    // Вставка элемента.
    // 1. Если такой ключ уже присутствует, вставка не производиться, возвращается итератор
    //    на уже присутствующий элемент и false.
    // 2. Если такого ключа ещё нет, производиться вставка, возвращается итератор на созданный
    //    элемент и true.
    // Если после вставки число элементов кеша превышает capacity, самый давно не
    // использованный элемент удаляется. Все итераторы на него инвалидируется.
    // Вставленный либо найденный с помощью этой функции элемент помечается как наиболее поздно
    // использованный.
    std::pair<iterator, bool> insert(value_type);

    // Удаление элемента.
    // Все итераторы на указанный элемент инвалидируются.
    void erase(iterator);

    // Возващает итератор на элемент с минимальный ключом.
    iterator begin() const;
    // Возващает итератор на элемент следующий за элементом с максимальным ключом.
    iterator end() const;

    size_t get_capacity() const;
    size_t get_size() const;

private:

    struct node
    {
        value_type val;
//        key_type key;
//        mapped_type mapped;

        node *left; //bst
        node *right;
        node *parent;

        node *next; //linked list
        node *prev;
    };

    node *root; //points to fake maximal root
    node *head; //head of cycled doubly linked list, points to the most least recently used node
    size_t capacity; //max size of bst
    size_t size;

    void recursive_node_remover(node *cur_node);

    std::pair<node *, bool> insert_(value_type);

    node *find_by_key_(key_type); //returns pointer to the required node, if it exists, else to its parent
    node *find_predessor(node *) const;
    node *find_successor(node *) const;
    node *find_min() const;

    void erase_(node *, bool); //if memory doesn't have to be freed, only pointers to this node become NULL
    void erase_no_child(node *);
    void erase_one_child(node *);
    void erase_two_child(node *);

    void list_cut_node_off(node *); //cuts node out from the list
    void list_node_to_head(
            node *); //makes a node new head (existing node is cut off from its place and moved to the beginning)
    void list_insert_node(node *); //inserts new node in list and makes it head
};

struct lru_cache::iterator
{
    friend struct lru_cache;

    iterator() : my(NULL), cur_cache(NULL)
    { };

    iterator(node *a, lru_cache const *cur) : my(a), cur_cache(cur)
    { };

    value_type const& operator*();

    iterator& operator++();
    iterator& operator--();

    iterator operator++(int);
    iterator operator--(int);

    bool operator==(const iterator rhs);
    bool operator!=(const iterator rhs);

private:
    node *my;
    lru_cache const *cur_cache;
};

#endif //LRU_CACHE_H
