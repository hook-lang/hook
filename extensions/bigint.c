//
// The Hook Programming Language
// bigint.c
//

#include "bigint.h"
#include <stdlib.h>
#include <gmp.h>
#include <hook/memory.h>
#include <hook/check.h>
#include <hook/status.h>

typedef struct
{
  HK_USERDATA_HEADER
  mpz_t num;
} BigInt;

static inline BigInt *bigint_new(mpz_t num);
static void bigint_deinit(HkUserdata *udata);
static int new_call(HkState *state, HkValue *args);
static int from_hex_call(HkState *state, HkValue *args);
static int to_string_call(HkState *state, HkValue *args);
static int add_call(HkState *state, HkValue *args);
static int sub_call(HkState *state, HkValue *args);
static int mul_call(HkState *state, HkValue *args);
static int div_call(HkState *state, HkValue *args);
static int mod_call(HkState *state, HkValue *args);
static int pow_call(HkState *state, HkValue *args);
static int sqrt_call(HkState *state, HkValue *args);
static int neg_call(HkState *state, HkValue *args);
static int abs_call(HkState *state, HkValue *args);
static int compare_call(HkState *state, HkValue *args);

static inline BigInt *bigint_new(mpz_t num)
{
  BigInt *bigint = (BigInt *) hk_allocate(sizeof(*bigint));
  hk_userdata_init((HkUserdata *) bigint, &bigint_deinit);
  mpz_init_set(bigint->num, num);
  return bigint;
}

static void bigint_deinit(HkUserdata *udata)
{
  mpz_clear(((BigInt *) udata)->num);
}

static int new_call(HkState *state, HkValue *args)
{
  HkType types[] = {HK_TYPE_NUMBER, HK_TYPE_STRING};
  if (hk_check_argument_types(args, 1, 2, types) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  HkValue val = args[1];
  mpz_t num;
  if (hk_is_number(val))
    mpz_init_set_d(num, hk_as_number(val));
  else
    mpz_init_set_str(num, hk_as_string(val)->chars, 10);
  BigInt *bigint = bigint_new(num);
  return hk_state_push_userdata(state, (HkUserdata *) bigint);
}

static int from_hex_call(HkState *state, HkValue *args)
{
  if (!hk_check_argument_string(args, 1))
    return HK_STATUS_ERROR;
  HkString *str = hk_as_string(args[1]);
  mpz_t num;
  mpz_init_set_str(num, str->chars, 16);
  BigInt *bigint = bigint_new(num);
  return hk_state_push_userdata(state, (HkUserdata *) bigint);
}

static int to_string_call(HkState *state, HkValue *args)
{
  if (hk_check_argument_userdata(args, 1) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  BigInt *bigint = (BigInt *) hk_as_userdata(args[1]);
  char *chars = mpz_get_str(NULL, 10, bigint->num);
  HkString *str = hk_string_from_chars(-1, chars);
  free(chars);
  return hk_state_push_string(state, str);
}

static int add_call(HkState *state, HkValue *args)
{
  if (hk_check_argument_userdata(args, 1) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_check_argument_userdata(args, 2) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  BigInt *bigint1 = (BigInt *) hk_as_userdata(args[1]);
  BigInt *bigint2 = (BigInt *) hk_as_userdata(args[2]);
  mpz_t result;
  mpz_init(result);
  mpz_add(result, bigint1->num, bigint2->num);
  BigInt *bigint = bigint_new(result);
  return hk_state_push_userdata(state, (HkUserdata *) bigint);
}

static int sub_call(HkState *state, HkValue *args)
{
  if (hk_check_argument_userdata(args, 1) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_check_argument_userdata(args, 2) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  BigInt *bigint1 = (BigInt *) hk_as_userdata(args[1]);
  BigInt *bigint2 = (BigInt *) hk_as_userdata(args[2]);
  mpz_t result;
  mpz_init(result);
  mpz_sub(result, bigint1->num, bigint2->num);
  BigInt *bigint = bigint_new(result);
  return hk_state_push_userdata(state, (HkUserdata *) bigint);
}

static int mul_call(HkState *state, HkValue *args)
{
  if (hk_check_argument_userdata(args, 1) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_check_argument_userdata(args, 2) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  BigInt *bigint1 = (BigInt *) hk_as_userdata(args[1]);
  BigInt *bigint2 = (BigInt *) hk_as_userdata(args[2]);
  mpz_t result;
  mpz_init(result);
  mpz_mul(result, bigint1->num, bigint2->num);
  BigInt *bigint = bigint_new(result);
  return hk_state_push_userdata(state, (HkUserdata *) bigint);
}

static int div_call(HkState *state, HkValue *args)
{
  if (hk_check_argument_userdata(args, 1) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_check_argument_userdata(args, 2) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  BigInt *bigint1 = (BigInt *) hk_as_userdata(args[1]);
  BigInt *bigint2 = (BigInt *) hk_as_userdata(args[2]);
  mpz_t result;
  mpz_init(result);
  mpz_tdiv_q(result, bigint1->num, bigint2->num);
  BigInt *bigint = bigint_new(result);
  return hk_state_push_userdata(state, (HkUserdata *) bigint);
}

static int mod_call(HkState *state, HkValue *args)
{
  if (hk_check_argument_userdata(args, 1) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_check_argument_userdata(args, 2) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  BigInt *bigint1 = (BigInt *) hk_as_userdata(args[1]);
  BigInt *bigint2 = (BigInt *) hk_as_userdata(args[2]);
  mpz_t result;
  mpz_init(result);
  mpz_tdiv_r(result, bigint1->num, bigint2->num);
  BigInt *bigint = bigint_new(result);
  return hk_state_push_userdata(state, (HkUserdata *) bigint);
}

static int pow_call(HkState *state, HkValue *args)
{
  if (hk_check_argument_userdata(args, 1) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_check_argument_userdata(args, 2) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  BigInt *bigint1 = (BigInt *) hk_as_userdata(args[1]);
  BigInt *bigint2 = (BigInt *) hk_as_userdata(args[2]);
  mpz_t result;
  mpz_init(result);
  mpz_pow_ui(result, bigint1->num, mpz_get_ui(bigint2->num));
  BigInt *bigint = bigint_new(result);
  return hk_state_push_userdata(state, (HkUserdata *) bigint);
}

static int sqrt_call(HkState *state, HkValue *args)
{
  if (hk_check_argument_userdata(args, 1) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  BigInt *bigint = (BigInt *) hk_as_userdata(args[1]);
  mpz_t result;
  mpz_init(result);
  mpz_sqrt(result, bigint->num);
  BigInt *bigint2 = bigint_new(result);
  return hk_state_push_userdata(state, (HkUserdata *) bigint2);
}

static int neg_call(HkState *state, HkValue *args)
{
  if (hk_check_argument_userdata(args, 1) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  BigInt *bigint = (BigInt *) hk_as_userdata(args[1]);
  mpz_t result;
  mpz_init(result);
  mpz_neg(result, bigint->num);
  BigInt *bigint2 = bigint_new(result);
  return hk_state_push_userdata(state, (HkUserdata *) bigint2);
}

static int abs_call(HkState *state, HkValue *args)
{
  if (hk_check_argument_userdata(args, 1) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  BigInt *bigint = (BigInt *) hk_as_userdata(args[1]);
  mpz_t result;
  mpz_init(result);
  mpz_abs(result, bigint->num);
  BigInt *bigint2 = bigint_new(result);
  return hk_state_push_userdata(state, (HkUserdata *) bigint2);
}

static int compare_call(HkState *state, HkValue *args)
{
  if (hk_check_argument_userdata(args, 1) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_check_argument_userdata(args, 2) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  BigInt *bigint1 = (BigInt *) hk_as_userdata(args[1]);
  BigInt *bigint2 = (BigInt *) hk_as_userdata(args[2]);
  int result = mpz_cmp(bigint1->num, bigint2->num);
  return hk_state_push_number(state, result);
}

HK_LOAD_FN(bigint)
{
  if (hk_state_push_string_from_chars(state, -1, "bigint") == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_state_push_string_from_chars(state, -1, "new") == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_state_push_new_native(state, "new", 1, &new_call) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_state_push_string_from_chars(state, -1, "from_hex") == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_state_push_new_native(state, "from_hex", 1, &from_hex_call) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_state_push_string_from_chars(state, -1, "to_string") == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_state_push_new_native(state, "to_string", 1, &to_string_call) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_state_push_string_from_chars(state, -1, "add") == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_state_push_new_native(state, "add", 2, &add_call) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_state_push_string_from_chars(state, -1, "sub") == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_state_push_new_native(state, "sub", 2, &sub_call) == HK_STATUS_ERROR)  
    return HK_STATUS_ERROR;
  if (hk_state_push_string_from_chars(state, -1, "mul") == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_state_push_new_native(state, "mul", 2, &mul_call) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_state_push_string_from_chars(state, -1, "div") == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_state_push_new_native(state, "div", 2, &div_call) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_state_push_string_from_chars(state, -1, "mod") == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_state_push_new_native(state, "mod", 2, &mod_call) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_state_push_string_from_chars(state, -1, "pow") == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_state_push_new_native(state, "pow", 2, &pow_call) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_state_push_string_from_chars(state, -1, "sqrt") == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_state_push_new_native(state, "sqrt", 1, &sqrt_call) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_state_push_string_from_chars(state, -1, "neg") == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_state_push_new_native(state, "neg", 1, &neg_call) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_state_push_string_from_chars(state, -1, "abs") == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_state_push_new_native(state, "abs", 1, &abs_call) == HK_STATUS_ERROR)  
    return HK_STATUS_ERROR;
  if (hk_state_push_string_from_chars(state, -1, "compare") == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_state_push_new_native(state, "compare", 2, &compare_call) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  return hk_state_construct(state, 13);
}
