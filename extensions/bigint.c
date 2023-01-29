//
// The Hook Programming Language
// bigint.c
//

#include "bigint.h"
#include <stdlib.h>
#include <gmp.h>
#include "hk_memory.h"
#include "hk_status.h"

typedef struct
{
  HK_USERDATA_HEADER
  mpz_t num;
} bigint_t;

static inline bigint_t *bigint_new(mpz_t num);
static void bigint_deinit(hk_userdata_t *udata);
static int32_t new_call(hk_vm_t *vm, hk_value_t *args);
static int32_t from_hex_call(hk_vm_t *vm, hk_value_t *args);
static int32_t to_string_call(hk_vm_t *vm, hk_value_t *args);
static int32_t add_call(hk_vm_t *vm, hk_value_t *args);
static int32_t sub_call(hk_vm_t *vm, hk_value_t *args);
static int32_t mul_call(hk_vm_t *vm, hk_value_t *args);
static int32_t div_call(hk_vm_t *vm, hk_value_t *args);
static int32_t mod_call(hk_vm_t *vm, hk_value_t *args);
static int32_t pow_call(hk_vm_t *vm, hk_value_t *args);
static int32_t neg_call(hk_vm_t *vm, hk_value_t *args);
static int32_t abs_call(hk_vm_t *vm, hk_value_t *args);

static inline bigint_t *bigint_new(mpz_t num)
{
  bigint_t *bigint = (bigint_t *) hk_allocate(sizeof(*bigint));
  hk_userdata_init((hk_userdata_t *) bigint, &bigint_deinit);
  mpz_init_set(bigint->num, num);
  return bigint;
}

static void bigint_deinit(hk_userdata_t *udata)
{
  mpz_clear(((bigint_t *) udata)->num);
}

static int32_t new_call(hk_vm_t *vm, hk_value_t *args)
{
  int32_t types[] = {HK_TYPE_NUMBER, HK_TYPE_STRING};
  if (hk_vm_check_types(args, 1, 2, types) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  hk_value_t val = args[1];
  mpz_t num;
  if (hk_is_number(val))
    mpz_init_set_d(num, hk_as_number(val));
  else
    mpz_init_set_str(num, hk_as_string(val)->chars, 10);
  bigint_t *bigint = bigint_new(num);
  return hk_vm_push_userdata(vm, (hk_userdata_t *) bigint);
}

static int32_t from_hex_call(hk_vm_t *vm, hk_value_t *args)
{
  if (!hk_vm_check_string(args, 1))
    return HK_STATUS_ERROR;
  hk_string_t *str = hk_as_string(args[1]);
  mpz_t num;
  mpz_init_set_str(num, str->chars, 16);
  bigint_t *bigint = bigint_new(num);
  return hk_vm_push_userdata(vm, (hk_userdata_t *) bigint);
}

static int32_t to_string_call(hk_vm_t *vm, hk_value_t *args)
{
  if (hk_vm_check_userdata(args, 1) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  bigint_t *bigint = (bigint_t *) hk_as_userdata(args[1]);
  char *chars = mpz_get_str(NULL, 10, bigint->num);
  hk_string_t *str = hk_string_from_chars(-1, chars);
  free(chars);
  return hk_vm_push_string(vm, str);
}

static int32_t add_call(hk_vm_t *vm, hk_value_t *args)
{
  if (hk_vm_check_userdata(args, 1) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_vm_check_userdata(args, 2) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  bigint_t *bigint1 = (bigint_t *) hk_as_userdata(args[1]);
  bigint_t *bigint2 = (bigint_t *) hk_as_userdata(args[2]);
  mpz_t result;
  mpz_init(result);
  mpz_add(result, bigint1->num, bigint2->num);
  bigint_t *bigint = bigint_new(result);
  return hk_vm_push_userdata(vm, (hk_userdata_t *) bigint);
}

static int32_t sub_call(hk_vm_t *vm, hk_value_t *args)
{
  if (hk_vm_check_userdata(args, 1) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_vm_check_userdata(args, 2) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  bigint_t *bigint1 = (bigint_t *) hk_as_userdata(args[1]);
  bigint_t *bigint2 = (bigint_t *) hk_as_userdata(args[2]);
  mpz_t result;
  mpz_init(result);
  mpz_sub(result, bigint1->num, bigint2->num);
  bigint_t *bigint = bigint_new(result);
  return hk_vm_push_userdata(vm, (hk_userdata_t *) bigint);
}

static int32_t mul_call(hk_vm_t *vm, hk_value_t *args)
{
  if (hk_vm_check_userdata(args, 1) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_vm_check_userdata(args, 2) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  bigint_t *bigint1 = (bigint_t *) hk_as_userdata(args[1]);
  bigint_t *bigint2 = (bigint_t *) hk_as_userdata(args[2]);
  mpz_t result;
  mpz_init(result);
  mpz_mul(result, bigint1->num, bigint2->num);
  bigint_t *bigint = bigint_new(result);
  return hk_vm_push_userdata(vm, (hk_userdata_t *) bigint);
}

static int32_t div_call(hk_vm_t *vm, hk_value_t *args)
{
  if (hk_vm_check_userdata(args, 1) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_vm_check_userdata(args, 2) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  bigint_t *bigint1 = (bigint_t *) hk_as_userdata(args[1]);
  bigint_t *bigint2 = (bigint_t *) hk_as_userdata(args[2]);
  mpz_t result;
  mpz_init(result);
  mpz_tdiv_q(result, bigint1->num, bigint2->num);
  bigint_t *bigint = bigint_new(result);
  return hk_vm_push_userdata(vm, (hk_userdata_t *) bigint);
}

static int32_t mod_call(hk_vm_t *vm, hk_value_t *args)
{
  if (hk_vm_check_userdata(args, 1) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_vm_check_userdata(args, 2) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  bigint_t *bigint1 = (bigint_t *) hk_as_userdata(args[1]);
  bigint_t *bigint2 = (bigint_t *) hk_as_userdata(args[2]);
  mpz_t result;
  mpz_init(result);
  mpz_tdiv_r(result, bigint1->num, bigint2->num);
  bigint_t *bigint = bigint_new(result);
  return hk_vm_push_userdata(vm, (hk_userdata_t *) bigint);
}

static int32_t pow_call(hk_vm_t *vm, hk_value_t *args)
{
  if (hk_vm_check_userdata(args, 1) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_vm_check_userdata(args, 2) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  bigint_t *bigint1 = (bigint_t *) hk_as_userdata(args[1]);
  bigint_t *bigint2 = (bigint_t *) hk_as_userdata(args[2]);
  mpz_t result;
  mpz_init(result);
  mpz_pow_ui(result, bigint1->num, mpz_get_ui(bigint2->num));
  bigint_t *bigint = bigint_new(result);
  return hk_vm_push_userdata(vm, (hk_userdata_t *) bigint);
}

static int32_t neg_call(hk_vm_t *vm, hk_value_t *args)
{
  if (hk_vm_check_userdata(args, 1) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  bigint_t *bigint = (bigint_t *) hk_as_userdata(args[1]);
  mpz_t result;
  mpz_init(result);
  mpz_neg(result, bigint->num);
  bigint_t *bigint2 = bigint_new(result);
  return hk_vm_push_userdata(vm, (hk_userdata_t *) bigint2);
}

static int32_t abs_call(hk_vm_t *vm, hk_value_t *args)
{
  if (hk_vm_check_userdata(args, 1) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  bigint_t *bigint = (bigint_t *) hk_as_userdata(args[1]);
  mpz_t result;
  mpz_init(result);
  mpz_abs(result, bigint->num);
  bigint_t *bigint2 = bigint_new(result);
  return hk_vm_push_userdata(vm, (hk_userdata_t *) bigint2);
}

HK_LOAD_FN(bigint)
{
  if (hk_vm_push_string_from_chars(vm, -1, "bigint") == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_vm_push_string_from_chars(vm, -1, "new") == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_vm_push_new_native(vm, "new", 1, &new_call) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_vm_push_string_from_chars(vm, -1, "from_hex") == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_vm_push_new_native(vm, "from_hex", 1, &from_hex_call) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_vm_push_string_from_chars(vm, -1, "to_string") == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_vm_push_new_native(vm, "to_string", 1, &to_string_call) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_vm_push_string_from_chars(vm, -1, "add") == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_vm_push_new_native(vm, "add", 2, &add_call) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_vm_push_string_from_chars(vm, -1, "sub") == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_vm_push_new_native(vm, "sub", 2, &sub_call) == HK_STATUS_ERROR)  
    return HK_STATUS_ERROR;
  if (hk_vm_push_string_from_chars(vm, -1, "mul") == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_vm_push_new_native(vm, "mul", 2, &mul_call) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_vm_push_string_from_chars(vm, -1, "div") == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_vm_push_new_native(vm, "div", 2, &div_call) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_vm_push_string_from_chars(vm, -1, "mod") == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_vm_push_new_native(vm, "mod", 2, &mod_call) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_vm_push_string_from_chars(vm, -1, "pow") == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_vm_push_new_native(vm, "pow", 2, &pow_call) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_vm_push_string_from_chars(vm, -1, "neg") == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_vm_push_new_native(vm, "neg", 1, &neg_call) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_vm_push_string_from_chars(vm, -1, "abs") == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_vm_push_new_native(vm, "abs", 1, &abs_call) == HK_STATUS_ERROR)  
    return HK_STATUS_ERROR;
  return hk_vm_construct(vm, 11);
}
