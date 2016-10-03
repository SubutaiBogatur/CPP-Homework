//
// Created by Aleksandr Tukallo on 25.09.16.
//

#include <assert.h>
#include "lru-cache.h"

lru_cache::lru_cache(size_t cap)
{
    assert(cap > 0);
    this->root = new node;
    this->root->parent = this->root->left = this->root->right = this->root->next = this->root->prev = NULL;
    root->val.first = 666;

    this->head = NULL;
    this->capacity = cap;
    this->size = 0;
}

lru_cache::~lru_cache()
{
    //root is always not NULL
    this->recursive_node_remover(root);
}

void lru_cache::recursive_node_remover(node *cur_node)
{
    if (cur_node->left != NULL)
    {
        recursive_node_remover(cur_node->left);
    }
    if (cur_node->right != NULL)
    {
        recursive_node_remover(cur_node->right);
    }
    delete cur_node;
}

lru_cache::node *lru_cache::find_by_key_(key_type key_to_find)
{
    node *cur_node = this->root;

    while (true)
    {
        if (cur_node == root || key_to_find < cur_node->val.first) //key to find is always less than root
        {
            //going to left subtree
            if (cur_node->left != NULL)
            {
                cur_node = cur_node->left;
                continue;
            } else
            {
                return cur_node;
            }
        } else if (key_to_find > cur_node->val.first)
        {
            //going to right subtree
            if (cur_node->right != NULL)
            {
                cur_node = cur_node->right;
                continue;
            } else
            {
                return cur_node;
            }
        } else
        {
            //current node has the key needed to find
            return cur_node;
        }
    }
}

lru_cache::node *lru_cache::find_min() const
{
    node *cur_node = this->root;
    while (cur_node->left != NULL)
    {
        cur_node = cur_node->left;
    }
    return cur_node;
}

lru_cache::node *lru_cache::find_successor(node *nod) const
{
    if (nod->right != NULL) //then go right and then always left
    {
        nod = nod->right;
        while (nod->left != NULL)
        {
            nod = nod->left;
        }
        return nod;
    }
    else //go up until we see left child ancestor
    {
        while (nod->parent != NULL && nod != nod->parent->left)
        {
            nod = nod->parent;
        }
        return nod->parent; //if nod is max element, fake root returned
    }
}

lru_cache::node *lru_cache::find_predessor(node *nod) const
{
    if (nod->left != NULL)
    {
        nod = nod->left;
        while (nod->right != NULL)
        {
            nod = nod->right;
        }
        return nod;
    }
    else
    {
        while (nod->parent != NULL && nod != nod->parent->right)
        {
            nod = nod->parent;
        }
        return nod->parent; //if nod is min element, NULL is returned
    }
}

void lru_cache::list_insert_node(node *head_new)
{
    if (head == NULL)
    {
        head_new->next = head_new->prev = head_new;
    }
    else
    {
        //making correct ptrs in head_new
        head_new->prev = this->head->prev;
        head_new->next = this->head;

        //inserting before old_head
        this->head->prev->next = head_new;
        this->head->prev = head_new;
    }

    this->head = head_new;
}

void lru_cache::list_node_to_head(node *head_new)
{
    list_cut_node_off(head_new);
    list_insert_node(head_new);
}

void lru_cache::list_cut_node_off(node *tmp)
{
    if (head == tmp)
    {
        head = tmp->next;
    }

    tmp->prev->next = tmp->next;
    tmp->next->prev = tmp->prev;
}

std::pair<lru_cache::node *, bool> lru_cache::insert_(value_type value)
{
    node *finded_node = this->find_by_key_(value.first);
    if (finded_node != root && finded_node->val.first == value.first) //such key already exists in tree
    {
        list_node_to_head(finded_node);
        return {finded_node, false};
    }

    node *ins_node;

    if (size < capacity)
    {
        ins_node = new node;
    } else
    {
        ins_node = head->prev;
        this->erase_(ins_node, false); //delete pointers to this node, but don't free memory
        finded_node = this->find_by_key_(value.first); //count once again for place to insert in new tree
    }

    //just inserting node in bst
    ins_node->val.first = value.first;
    ins_node->val.second = value.second;
    ins_node->left = ins_node->right = NULL;
    ins_node->parent = finded_node;

    if (finded_node == root || ins_node->val.first < finded_node->val.first)
    {
        finded_node->left = ins_node;
    } else
    {
        finded_node->right = ins_node;
    }

    list_insert_node(ins_node);
    if (size < capacity)
    {
        size++;
    }

    return {ins_node, true};
}

void lru_cache::erase_no_child(node *del_node)
{
    assert(del_node != root);

    if (del_node->parent == root || del_node->val.first < del_node->parent->val.first)
    {
        del_node->parent->left = NULL;
    }
    else
    {
        del_node->parent->right = NULL;
    }

}

void lru_cache::erase_one_child(node *del_node)
{
    assert(del_node != root);

    node *ptr_to_child;
    if (del_node->left != NULL)
    {
        ptr_to_child = del_node->left;
    }
    else
    {
        ptr_to_child = del_node->right;
    }

    ptr_to_child->parent = del_node->parent;

    if (del_node->parent == root || del_node->val.first < del_node->parent->val.first)
    {
        del_node->parent->left = ptr_to_child;
    }
    else
    {
        del_node->parent->right = ptr_to_child;
    }

}

void lru_cache::erase_two_child(node *del_node)
{
    node *pred = this->find_predessor(del_node);
    assert(pred != NULL);
    assert(del_node != root);

    //pred->right is always NULL

    if (pred->left != NULL)
    {
        erase_one_child(pred);
    }
    else
    {
        erase_no_child(pred);
    }

    pred->parent = del_node->parent;
    pred->left = del_node->left;
    pred->right = del_node->right;

    if (del_node->parent->left == del_node)
    {
        del_node->parent->left = pred;
    }
    else if (del_node->parent->right == del_node)
    {
        del_node->parent->right = pred;
    }

    if (del_node->left != NULL)
    {
        del_node->left->parent = pred;
    }
    if (del_node->right != NULL)
    {
        del_node->right->parent = pred;
    }
}

void lru_cache::erase_(node *del_node, bool free_memory)
{
    assert(del_node != root); //if root is deleted, the element they want to delete is not in the tree
    list_cut_node_off(del_node);
    if (del_node->left == NULL && del_node->right == NULL) //no children
    {
        erase_no_child(del_node);
    }
    else if (del_node->left != NULL && del_node->right != NULL) //2 children
    {
        erase_two_child(del_node);
    }
    else //1 child
    {
        erase_one_child(del_node);
    }

    if (free_memory)
    {
        delete del_node;
        size--;
    }
    return;
}

lru_cache::iterator lru_cache::find(key_type key)
{
    node *finded_node = this->find_by_key_(key);

    if (finded_node != root && finded_node->val.first == key)
    {
        this->list_node_to_head(finded_node);
        return iterator(finded_node, this);
    }
    else
        return this->end();
}

lru_cache::iterator lru_cache::begin() const
{
    return iterator(this->find_min(), this);
}

lru_cache::iterator lru_cache::end() const
{
    return iterator(this->root, this);
}

std::pair<lru_cache::iterator, bool> lru_cache::insert(value_type value)
{
    std::pair<lru_cache::node *, bool> ret = this->insert_(value);
    return {iterator(ret.first, this), ret.second};
}

void lru_cache::erase(lru_cache::iterator it)
{
    this->erase_(it.my, true);
    return;
}

value_type const& lru_cache::iterator::operator*()
{
    assert(*this != this->cur_cache->end());
    return this->my->val;
}

bool lru_cache::iterator::operator==(const iterator rhs)
{
    if (this->my == rhs.my && this->cur_cache == rhs.cur_cache)
        return true;
    else return false;
}

bool lru_cache::iterator::operator!=(const iterator rhs)
{
    return !(*this == rhs);
}

lru_cache::iterator& lru_cache::iterator::operator++()
{
    this->my = this->cur_cache->find_successor(this->my);
    return *this;
}

lru_cache::iterator& lru_cache::iterator::operator--()
{
    this->my = this->cur_cache->find_predessor(this->my);
    return *this;
}

lru_cache::iterator lru_cache::iterator::operator++(int)
{
    lru_cache::iterator res(this->my, this->cur_cache);
    ++(*this);
    return res;
}

lru_cache::iterator lru_cache::iterator::operator--(int)
{
    lru_cache::iterator res(this->my, this->cur_cache);
    --(*this);
    return res;
}

size_t lru_cache::get_capacity() const
{
    return capacity;
}

size_t lru_cache::get_size() const
{
    return size;
}
