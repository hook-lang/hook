//
// The Hook Programming Language
// bigint.c
//

#include "bigint.h"
#include <stdlib.h>
#include <gmp.h>

typedef struct
{
  HK_USERDATA_HEADER
  mpz_t num;
} BigInt;

static inline BigInt *bigint_new(mpz_t num);
static void bigint_deinit(HkUserdata *udata);
static void new_call(HkState *state, HkValue *args);
static void from_hex_call(HkState *state, HkValue *args);
static void to_string_call(HkState *state, HkValue *args);
static void add_call(HkState *state, HkValue *args);
static void sub_call(HkState *state, HkValue *args);
static void mul_call(HkState *state, HkValue *args);
static void div_call(HkState *state, HkValue *args);
static void mod_call(HkState *state, HkValue *args);
static void pow_call(HkState *state, HkValue *args);
static void sqrt_call(HkState *state, HkValue *args);
static void neg_call(HkState *state, HkValue *args);
static void abs_call(HkState *state, HkValue *args);
static void compare_call(HkState *state, HkValue *args);

static inline BigInt *bigint_new(mpz_t num)
{
  BigInt *bigint = (BigInt *) hk_allocate(sizeof(*bigint));
  hk_userdata_init((HkUserdata *) bigint, bigint_deinit);
  mpz_init_set(bigint->num, num);
  return bigint;
}

static void bigint_deinit(HkUserdata *udata)
{
  mpz_clear(((BigInt *) udata)->num);
}

static void new_call(HkState *state, HkValue *args)
{
  HkType types[] = {HK_TYPE_NUMBER, HK_TYPE_STRING};
  hk_state_check_argument_types(state, args, 1, 2, types);
  hk_return_if_not_ok(state);
  HkValue val = args[1];
  mpz_t num;
  if (hk_is_number(val))
    mpz_init_set_d(num, hk_as_number(val));
  else
    mpz_init_set_str(num, hk_as_string(val)->chars, 10);
  BigInt *bigint = bigint_new(num);
  hk_state_push_userdata(state, (HkUserdata *) bigint);
}

static void from_hex_call(HkState *state, HkValue *args)
{
  hk_state_check_argument_string(state, args, 1);
  hk_return_if_not_ok(state);
  HkString *str = hk_as_string(args[1]);
  mpz_t num;
  mpz_init_set_str(num, str->chars, 16);
  BigInt *bigint = bigint_new(num);
  hk_state_push_userdata(state, (HkUserdata *) bigint);
}

static void to_string_call(HkState *state, HkValue *args)
{
  hk_state_check_argument_userdata(state, args, 1);
  hk_return_if_not_ok(state);
  BigInt *bigint = (BigInt *) hk_as_userdata(args[1]);
  char *chars = mpz_get_str(NULL, 10, bigint->num);
  HkString *str = hk_string_from_chars(-1, chars);
  free(chars);
  hk_state_push_string(state, str);
}

static void add_call(HkState *state, HkValue *args)
{
  hk_state_check_argument_userdata(state, args, 1);
  hk_return_if_not_ok(state);
  hk_state_check_argument_userdata(state, args, 2);
  hk_return_if_not_ok(state);
  BigInt *bigint1 = (BigInt *) hk_as_userdata(args[1]);
  BigInt *bigint2 = (BigInt *) hk_as_userdata(args[2]);
  mpz_t result;
  mpz_init(result);
  mpz_add(result, bigint1->num, bigint2->num);
  BigInt *bigint = bigint_new(result);
  hk_state_push_userdata(state, (HkUserdata *) bigint);
}

static void sub_call(HkState *state, HkValue *args)
{
  hk_state_check_argument_userdata(state, args, 1);
  hk_return_if_not_ok(state);
  hk_state_check_argument_userdata(state, args, 2);
  hk_return_if_not_ok(state);
  BigInt *bigint1 = (BigInt *) hk_as_userdata(args[1]);
  BigInt *bigint2 = (BigInt *) hk_as_userdata(args[2]);
  mpz_t result;
  mpz_init(result);
  mpz_sub(result, bigint1->num, bigint2->num);
  BigInt *bigint = bigint_new(result);
  hk_state_push_userdata(state, (HkUserdata *) bigint);
}

static void mul_call(HkState *state, HkValue *args)
{
  hk_state_check_argument_userdata(state, args, 1);
  hk_return_if_not_ok(state);
  hk_state_check_argument_userdata(state, args, 2);
  hk_return_if_not_ok(state);
  BigInt *bigint1 = (BigInt *) hk_as_userdata(args[1]);
  BigInt *bigint2 = (BigInt *) hk_as_userdata(args[2]);
  mpz_t result;
  mpz_init(result);
  mpz_mul(result, bigint1->num, bigint2->num);
  BigInt *bigint = bigint_new(result);
  hk_state_push_userdata(state, (HkUserdata *) bigint);
}

static void div_call(HkState *state, HkValue *args)
{
  hk_state_check_argument_userdata(state, args, 1);
  hk_return_if_not_ok(state);
  hk_state_check_argument_userdata(state, args, 2);
  hk_return_if_not_ok(state);
  BigInt *bigint1 = (BigInt *) hk_as_userdata(args[1]);
  BigInt *bigint2 = (BigInt *) hk_as_userdata(args[2]);
  mpz_t result;
  mpz_init(result);
  mpz_tdiv_q(result, bigint1->num, bigint2->num);
  BigInt *bigint = bigint_new(result);
  hk_state_push_userdata(state, (HkUserdata *) bigint);
}

static void mod_call(HkState *state, HkValue *args)
{
  hk_state_check_argument_userdata(state, args, 1);
  hk_return_if_not_ok(state);
  hk_state_check_argument_userdata(state, args, 2);
  hk_return_if_not_ok(state);
  BigInt *bigint1 = (BigInt *) hk_as_userdata(args[1]);
  BigInt *bigint2 = (BigInt *) hk_as_userdata(args[2]);
  mpz_t result;
  mpz_init(result);
  mpz_tdiv_r(result, bigint1->num, bigint2->num);
  BigInt *bigint = bigint_new(result);
  hk_state_push_userdata(state, (HkUserdata *) bigint);
}

static void pow_call(HkState *state, HkValue *args)
{
  hk_state_check_argument_userdata(state, args, 1);
  hk_return_if_not_ok(state);
  hk_state_check_argument_userdata(state, args, 2);
  hk_return_if_not_ok(state);
  BigInt *bigint1 = (BigInt *) hk_as_userdata(args[1]);
  BigInt *bigint2 = (BigInt *) hk_as_userdata(args[2]);
  mpz_t result;
  mpz_init(result);
  mpz_pow_ui(result, bigint1->num, mpz_get_ui(bigint2->num));
  BigInt *bigint = bigint_new(result);
  hk_state_push_userdata(state, (HkUserdata *) bigint);
}

static void sqrt_call(HkState *state, HkValue *args)
{
  hk_state_check_argument_userdata(state, args, 1);
  hk_return_if_not_ok(state);
  BigInt *bigint = (BigInt *) hk_as_userdata(args[1]);
  mpz_t result;
  mpz_init(result);
  mpz_sqrt(result, bigint->num);
  BigInt *bigint2 = bigint_new(result);
  hk_state_push_userdata(state, (HkUserdata *) bigint2);
}

static void neg_call(HkState *state, HkValue *args)
{
  hk_state_check_argument_userdata(state, args, 1);
  hk_return_if_not_ok(state);
  BigInt *bigint = (BigInt *) hk_as_userdata(args[1]);
  mpz_t result;
  mpz_init(result);
  mpz_neg(result, bigint->num);
  BigInt *bigint2 = bigint_new(result);
  hk_state_push_userdata(state, (HkUserdata *) bigint2);
}

static void abs_call(HkState *state, HkValue *args)
{
  hk_state_check_argument_userdata(state, args, 1);
  hk_return_if_not_ok(state);
  BigInt *bigint = (BigInt *) hk_as_userdata(args[1]);
  mpz_t result;
  mpz_init(result);
  mpz_abs(result, bigint->num);
  BigInt *bigint2 = bigint_new(result);
  hk_state_push_userdata(state, (HkUserdata *) bigint2);
}

static void compare_call(HkState *state, HkValue *args)
{
  hk_state_check_argument_userdata(state, args, 1);
  hk_return_if_not_ok(state);
  hk_state_check_argument_userdata(state, args, 2);
  hk_return_if_not_ok(state);
  BigInt *bigint1 = (BigInt *) hk_as_userdata(args[1]);
  BigInt *bigint2 = (BigInt *) hk_as_userdata(args[2]);
  int result = mpz_cmp(bigint1->num, bigint2->num);
  hk_state_push_number(state, result);
}

HK_LOAD_MODULE_HANDLER(bigint)
{
  hk_state_push_string_from_chars(state, -1, "bigint");
  hk_return_if_not_ok(state);
  hk_state_push_string_from_chars(state, -1, "new");
  hk_return_if_not_ok(state);
  hk_state_push_new_native(state, "new", 1, new_call);
  hk_return_if_not_ok(state);
  hk_state_push_string_from_chars(state, -1, "from_hex");
  hk_return_if_not_ok(state);
  hk_state_push_new_native(state, "from_hex", 1, from_hex_call);
  hk_return_if_not_ok(state);
  hk_state_push_string_from_chars(state, -1, "to_string");
  hk_return_if_not_ok(state);
  hk_state_push_new_native(state, "to_string", 1, to_string_call);
  hk_return_if_not_ok(state);
  hk_state_push_string_from_chars(state, -1, "add");
  hk_return_if_not_ok(state);
  hk_state_push_new_native(state, "add", 2, add_call);
  hk_return_if_not_ok(state);
  hk_state_push_string_from_chars(state, -1, "sub");
  hk_return_if_not_ok(state);
  hk_state_push_new_native(state, "sub", 2, sub_call);
  hk_return_if_not_ok(state);
  hk_state_push_string_from_chars(state, -1, "mul");
  hk_return_if_not_ok(state);
  hk_state_push_new_native(state, "mul", 2, mul_call);
  hk_return_if_not_ok(state);
  hk_state_push_string_from_chars(state, -1, "div");
  hk_return_if_not_ok(state);
  hk_state_push_new_native(state, "div", 2, div_call);
  hk_return_if_not_ok(state);
  hk_state_push_string_from_chars(state, -1, "mod");
  hk_return_if_not_ok(state);
  hk_state_push_new_native(state, "mod", 2, mod_call);
  hk_return_if_not_ok(state);
  hk_state_push_string_from_chars(state, -1, "pow");
  hk_return_if_not_ok(state);
  hk_state_push_new_native(state, "pow", 2, pow_call);
  hk_return_if_not_ok(state);
  hk_state_push_string_from_chars(state, -1, "sqrt");
  hk_return_if_not_ok(state);
  hk_state_push_new_native(state, "sqrt", 1, sqrt_call);
  hk_return_if_not_ok(state);
  hk_state_push_string_from_chars(state, -1, "neg");
  hk_return_if_not_ok(state);
  hk_state_push_new_native(state, "neg", 1, neg_call);
  hk_return_if_not_ok(state);
  hk_state_push_string_from_chars(state, -1, "abs");
  hk_return_if_not_ok(state);
  hk_state_push_new_native(state, "abs", 1, abs_call);
  hk_return_if_not_ok(state);
  hk_state_push_string_from_chars(state, -1, "compare");
  hk_return_if_not_ok(state);
  hk_state_push_new_native(state, "compare", 2, compare_call);
  hk_return_if_not_ok(state);
  hk_state_construct(state, 13);
}
