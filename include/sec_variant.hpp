//
// Created by zhang on 6/3/18.
//

#ifndef SECFT_SEC_VARIANT_H
#define SECFT_SEC_VARIANT_H

#include <cstdint>
#include <cstring>

#ifdef __cplusplus
extern "C" {
#endif

/* warning: variant is thread unsafe, avoid use in multi-thread */
typedef struct sec_variant_s {
    union {
        bool bool_val;
        uint8_t uint8_val;
        int16_t int16_val;
        uint16_t uint16_val;
        int32_t int32_val;
        uint32_t uint32_val;
        int64_t int64_val;
        uint64_t uint64_val;
        float float_val;
        double double_val;
        void *ptr_val;
        char *str_val;
    };

    char vt;  /* variant type */
    uint16_t len; /* variant buffer len */
} sec_variant_t;

/*********************************************************************
* Pre-defined variant types.
*********************************************************************/
typedef enum {
    vtype_null = 0,
    vtype_bool = 'b',
    vtype_uint8 = 'y',
    vtype_int16 = 'n',
    vtype_uint16 = 'q',
    vtype_int32 = 'i',
    vtype_uint32 = 'u',
    vtype_int64 = 'x',
    vtype_uint64 = 't',
    vtype_float = 'f',
    vtype_double = 'd',
    vtype_string = 'c',
    vtype_ptr = '*',
    vtype_autoptr = 'a',  /* internal use only */
    vtype_array = '[',
    vtype_dict = '{',
    vtype_variant = 'v',
} sec_variant_type;

/* string alias for variant types */
#ifndef vbool
# define vbool             "b"
#endif
#ifndef vuint8
# define vuint8            "y"
#endif
#ifndef vint16
# define vint16            "n"
#endif
#ifndef vuint16
# define vuint16           "q"
#endif
#ifndef vint32
# define vint32            "i"
#endif
#ifndef vuint32
# define vuint32           "u"
#endif
#ifndef vint64
# define vint64            "x"
#endif
#ifndef vuint64
# define vuint64           "t"
#endif
#ifndef vfloat
# define vfloat            "f"
#endif
#ifndef vdouble
# define vdouble           "d"
#endif
#ifndef vstring
# define vstring           "c"
#endif
#ifndef vptr
# define vptr              "*"
#endif
#ifndef vautoptr
# define vautoptr          "a"    /* internal use only */
#endif
#ifndef varray
# define varray            "["
#endif
#ifndef vdict
# define vdict             "{"
#endif
#ifndef vvariant
# define vvariant          "v"
#endif

#define vbool_new             sec_variant_bool_new
#define vuint8_new            sec_variant_uint8_new
#define vint16_new            sec_variant_int16_new
#define vuint16_new           sec_variant_uint16_new
#define vint32_new            sec_variant_int32_new
#define vuint32_new           sec_variant_uint32_new
#define vint64_new            sec_variant_int64_new
#define vuint64_new           sec_variant_uint64_new
#define vfloat_new            sec_variant_float_new
#define vdouble_new           sec_variant_double_new
#define vstr_new              sec_variant_str_new
#define vptr_new              sec_variant_ptr_new
#define vautoptr_new          sec_variant_autoptr_new
#define varray_new            sec_variant_array_new
#define vdict_new             sec_variant_dict_new

#define vbool_set             sec_variant_bool_set
#define vuint8_set            sec_variant_uint8_set
#define vint16_set            sec_variant_int16_set
#define vuint16_set           sec_variant_uint16_set
#define vint32_set            sec_variant_int32_set
#define vuint32_set           sec_variant_uint32_set
#define vint64_set            sec_variant_int64_set
#define vuint64_set           sec_variant_uint64_set
#define vfloat_set            sec_variant_float_set
#define vdouble_set           sec_variant_double_set
#define vstr_set              sec_variant_str_set
#define vptr_set              sec_variant_ptr_set
#define vautoptr_set          sec_variant_autoptr_set

sec_variant_t* sec_variant_bool_new(bool value){
    sec_variant_t *bool_variant = new sec_variant_t();
    bool_variant->vt = sec_variant_type::vtype_bool;
    bool_variant->len = 1;
    bool_variant->bool_val = value;
    return bool_variant;
}
sec_variant_t* sec_variant_bool_set(sec_variant_t* v, bool value){
    if(v->vt != sec_variant_type::vtype_bool){
        return sec_variant_bool_new(value);
    }
    v->bool_val = value;
    return v;
}
bool sec_variant_bool_get(sec_variant_t* v){
    if(!v->len){
        return false;
    }
    if(v->vt != sec_variant_type::vtype_bool) {
        return false;
    }
    return v->bool_val;
}

sec_variant_t* sec_variant_uint8_new(uint8_t value) {
    sec_variant_t *int8_variant = new sec_variant_t();
    int8_variant->vt = sec_variant_type::vtype_uint8;
    int8_variant->len = 1;
    int8_variant->uint8_val = value;
    return int8_variant;
}

sec_variant_t* sec_variant_uint8_set(sec_variant_t* v, uint8_t value){
    if(v->vt != sec_variant_type::vtype_uint8){
        delete v;
        return sec_variant_uint8_new(value);
    }
    v->uint8_val = value;
    return v;
}

uint8_t sec_variant_uint8_get(sec_variant_t* v){
    if(!v->len){
        return 0;
    }
    if(v->vt != sec_variant_type::vtype_uint8) {
        return 0;
    }
    return v->uint8_val;
}


sec_variant_t* sec_variant_int16_new(int16_t value)
{
    sec_variant_t *int16_variant = new sec_variant_t();
    int16_variant->vt = sec_variant_type::vtype_int16;
    int16_variant->len = 1;
    int16_variant->int16_val = value;
    return int16_variant;
}

sec_variant_t* sec_variant_int16_set(sec_variant_t* v, int16_t value)
{
    if(v->vt != sec_variant_type::vtype_int16){
        delete v;
        return sec_variant_int16_new(value);
    }
    v->int16_val = value;
    return v;
}

int16_t sec_variant_int16_get(sec_variant_t* v)
{
    if(!v->len){
        return 0;
    }
    if(v->vt != sec_variant_type::vtype_int16) {
        return 0;
    }
    return v->int16_val;
}


sec_variant_t* sec_variant_uint16_new(uint16_t value)
{
    sec_variant_t *uint16_variant = new sec_variant_t();
    uint16_variant->vt = sec_variant_type::vtype_uint16;
    uint16_variant->len = 1;
    uint16_variant->uint16_val = value;
    return uint16_variant;
}
sec_variant_t* sec_variant_uint16_set(sec_variant_t* v,  uint16_t value)
{
    if(v->vt != sec_variant_type::vtype_uint16){
        delete v;
        return sec_variant_uint16_new(value);
    }
    v->uint16_val = value;
    return v;
}
uint16_t sec_variant_uint16_get(sec_variant_t* v)
{
    if(!v->len){
        return 0;
    }
    if(v->vt != sec_variant_type::vtype_uint16) {
        return 0;
    }
    return v->uint16_val;
}

sec_variant_t* sec_variant_int32_new(int32_t value)
{
    sec_variant_t *int32_variant = new sec_variant_t();
    int32_variant->vt = sec_variant_type::vtype_int32;
    int32_variant->len = 1;
    int32_variant->int32_val = value;
    return int32_variant;
}
sec_variant_t* sec_variant_int32_set(sec_variant_t* v, int32_t value)
{
    if(v->vt != sec_variant_type::vtype_int32){
        delete v;
        return sec_variant_int32_new(value);
    }
    v->int32_val = value;
    return v;
}
int32_t sec_variant_int32_get(sec_variant_t* v)
{
    if(!v->len){
        return 0;
    }
    if(v->vt != sec_variant_type::vtype_int32) {
        return 0;
    }
    return v->int32_val;
}

sec_variant_t* sec_variant_uint32_new(uint32_t value)
{
    sec_variant_t *uint32_variant = new sec_variant_t();
    uint32_variant->vt = sec_variant_type::vtype_uint32;
    uint32_variant->len = 1;
    uint32_variant->uint32_val = value;
    return uint32_variant;
}
sec_variant_t* sec_variant_uint32_set(sec_variant_t* v, uint32_t value)
{
    if(v->vt != sec_variant_type::vtype_uint32){
        delete v;
        return sec_variant_uint32_new(value);
    }
    v->uint32_val = value;
    return v;
}
uint32_t sec_variant_uint32_get(sec_variant_t* v)
{
    if(!v->len){
        return 0;
    }
    if(v->vt != sec_variant_type::vtype_uint32) {
        return 0;
    }
    return v->uint32_val;
}

sec_variant_t* sec_variant_int64_new(int64_t value)
{
    sec_variant_t *int64_variant = new sec_variant_t();
    int64_variant->vt = sec_variant_type::vtype_int64;
    int64_variant->len = 1;
    int64_variant->int64_val = value;
    return int64_variant;
}
sec_variant_t* sec_variant_int64_set(sec_variant_t* v, int64_t value)
{
    if(v->vt != sec_variant_type::vtype_int64){
        delete v;
        return sec_variant_int64_new(value);
    }
    v->int64_val = value;
    return v;
}
int64_t sec_variant_int64_get(sec_variant_t* v)
{
    if(!v->len){
        return 0;
    }
    if(v->vt != sec_variant_type::vtype_int64) {
        return 0;
    }
    return v->int64_val;
}

sec_variant_t* sec_variant_uint64_new(uint64_t value)
{
    sec_variant_t *uint64_variant = new sec_variant_t();
    uint64_variant->vt = sec_variant_type::vtype_uint64;
    uint64_variant->len = 1;
    uint64_variant->uint64_val = value;
    return uint64_variant;
}
sec_variant_t* sec_variant_uint64_set(sec_variant_t* v, uint64_t value)
{
    if(v->vt != sec_variant_type::vtype_uint64){
        delete v;
        return sec_variant_uint64_new(value);
    }
    v->uint64_val = value;
    return v;
}
uint64_t sec_variant_uint64_get(sec_variant_t* v)
{
    if(!v->len){
        return 0;
    }
    if(v->vt != sec_variant_type::vtype_uint64) {
        return 0;
    }
    return v->uint64_val;
}

sec_variant_t* sec_variant_float_new(float value)
{
    sec_variant_t *float_variant = new sec_variant_t();
    float_variant->vt = sec_variant_type::vtype_float;
    float_variant->len = 1;
    float_variant->float_val = value;
    return float_variant;
}
sec_variant_t* sec_variant_float_set(sec_variant_t* v, float value)
{
    if(v->vt != sec_variant_type::vtype_float){
        delete v;
        return sec_variant_float_new(value);
    }
    v->float_val = value;
    return v;
}
float sec_variant_float_get(sec_variant_t* v)
{
    if(!v->len){
        return 0;
    }
    if(v->vt != sec_variant_type::vtype_float) {
        return 0;
    }
    return v->float_val;
}

sec_variant_t* sec_variant_double_new(double value)
{
    sec_variant_t *double_variant = new sec_variant_t();
    double_variant->vt = sec_variant_type::vtype_double;
    double_variant->len = 1;
    double_variant->double_val = value;
    return double_variant;
}
sec_variant_t* sec_variant_doble_set(sec_variant_t* v, double value)
{
    if(v->vt != sec_variant_type::vtype_double){
        delete v;
        return sec_variant_double_new(value);
    }
    v->double_val = value;
    return v;
}
float sec_variant_double_get(sec_variant_t* v)
{
    if(!v->len){
        return 0;
    }
    if(v->vt != sec_variant_type::vtype_double) {
        return 0;
    }
    return v->double_val;
}

sec_variant_t* sec_variant_str_new(const char* value)
{
    sec_variant_t *str_variant = new sec_variant_t();
    str_variant->vt = sec_variant_type::vtype_string;
    str_variant->len = strlen(value);
    char* str = new char[strlen(value) + 1];
    strcpy(str, value);
    str_variant->str_val = str;
    return str_variant;
}
sec_variant_t* sec_variant_str_set(sec_variant_t* v, char* value)
{
    if(v->vt != sec_variant_type::vtype_string){
        delete v;
        return sec_variant_str_new(value);
    }
    v->len = strlen(value);
    char* str = new char[strlen(value) + 1];
    strcpy(str, value);
    v->str_val = str;
    return v;
}
char* sec_variant_str_get(sec_variant_t* v)
{
    if(v->vt != sec_variant_type::vtype_string) {
        return nullptr;
    }
    return v->str_val;
}

sec_variant_t* sec_variant_ptr_new(void* value)
{
    sec_variant_t *ptr_variant = new sec_variant_t();
    ptr_variant->vt = sec_variant_type::vtype_ptr;
    ptr_variant->len = value?0:1;
    ptr_variant->ptr_val = value;
    return ptr_variant;
}
sec_variant_t* sec_variant_ptr_set(sec_variant_t* v, void* value)
{
    if(v->vt != sec_variant_type::vtype_ptr){
        delete v;
        return sec_variant_ptr_new(value);
    }
    v->len = value?0:1;
    v->ptr_val = value;
    return v;
}
void* sec_variant_ptr_get(sec_variant_t* v)
{
    if(!v->len){
        return nullptr;
    }
    if(v->vt != sec_variant_type::vtype_ptr) {
        return nullptr;
    }
    return v->str_val;
}

#ifdef __cplusplus
}
#endif
#endif //SECFT_SEC_VARIANT_H
