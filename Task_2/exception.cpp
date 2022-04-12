#include "exception.h"

jmp_buf_list::jmp_buf_list() : objects(nullptr), prev(root)
{
	root = this;
}

jmp_buf_list::~jmp_buf_list()
{
    std::cout << "~jmp_buf_list\n";
	root = prev;
}

companion_item_t::companion_item_t()
{
    if (root != nullptr)
    {
        prev = root->objects;
        root->objects = this;
    }
}

companion_item_t::~companion_item_t()
{
    if (root != nullptr)
    {
        root->objects = prev;
    }
    std::cout << "~companion_item_t\n";
}

void my_throw(ERROR error_code)
{
    if (root == nullptr || is_stack_cleaning_active) std::terminate();
    jmp_buf_list* node = root;
    companion_item_t* object = root->objects;

    is_stack_cleaning_active = true;

    while (object != nullptr)
    {
        companion_item_t* tmp = object;
        object = object->prev;
    	tmp->~companion_item_t();
    }

    is_stack_cleaning_active = false;
    
    longjmp(node->buffer, error_code);
}
