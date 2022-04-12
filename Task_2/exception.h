#pragma once
#include <iostream>
#include <csetjmp>



enum ERROR {
    BAD_FILE = 1,
    OUT_OF_MEMORY,
    DIVIDE_OR_MOD_BY_ZERO,
    EXCEPTION
};

struct companion_item_t {
    companion_item_t();
    virtual ~companion_item_t();
    companion_item_t* prev;
};

struct jmp_buf_list {
    jmp_buf_list();
    ~jmp_buf_list();
    jmp_buf         buffer;
    jmp_buf_list* prev;
    companion_item_t*      objects;
};

static jmp_buf_list* root = nullptr;
static bool is_stack_cleaning_active = false;

void my_throw(ERROR error_code);



#define CHECK_TYPE(var, type) { decltype(var)* __tmp; __tmp = (type *)nullptr; }

#define TRY \
  jmp_buf_list* __node = new jmp_buf_list(); \
  int __err = setjmp (__node->buffer); \
  if (__err != 0) delete __node; \
  if (__err == 0)

#define CATCH_ALL if (__err != 0)
#define CATCH(error_code) CHECK_TYPE(error_code, ERROR); if (__err == error_code)

#define RETHROW my_throw (ERROR(__err))
#define THROW(err) CHECK_TYPE(err, ERROR); my_throw (err)
