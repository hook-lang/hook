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

static inline void sqrtm_prime(mpz_t r, mpz_t a, mpz_t p);
static inline BigInt *bigint_new(void);
static void bigint_deinit(HkUserdata *udata);
static void new_call(HkState *state, HkValue *args);
static void from_string_call(HkState *state, HkValue *args);
static void to_string_call(HkState *state, HkValue *args);
static void from_bytes_call(HkState *state, HkValue *args);
static void to_bytes_call(HkState *state, HkValue *args);
static void sign_call(HkState *state, HkValue *args);
static void add_call(HkState *state, HkValue *args);
static void sub_call(HkState *state, HkValue *args);
static void mul_call(HkState *state, HkValue *args);
static void div_call(HkState *state, HkValue *args);
static void mod_call(HkState *state, HkValue *args);
static void pow_call(HkState *state, HkValue *args);
static void powm_call(HkState *state, HkValue *args);
static void sqrt_call(HkState *state, HkValue *args);
static void sqrtm_prime_call(HkState *state, HkValue *args);
static void neg_call(HkState *state, HkValue *args);
static void abs_call(HkState *state, HkValue *args);
static void compare_call(HkState *state, HkValue *args);
static void invertm_call(HkState *state, HkValue *args);
static void size_call(HkState *state, HkValue *args);
static void testbit_call(HkState *state, HkValue *args);

static inline void sqrtm_prime(mpz_t r, mpz_t a, mpz_t p)
{
  mpz_t t;
  mpz_init(t);
  mpz_add_ui(t, p, 1);
  mpz_div_ui(t, t, 4);
  mpz_powm(r, a, t, p);
  mpz_clear(t);
}

static inline BigInt *bigint_new(void)
{
  BigInt *bigint = (BigInt *) hk_allocate(sizeof(*bigint));
  hk_userdata_init((HkUserdata *) bigint, bigint_deinit);
  mpz_init(bigint->num);
  return bigint;
}

static void bigint_deinit(HkUserdata *udata)
{
  mpz_clear(((BigInt *) udata)->num);
}

static void new_call(HkState *state, HkValue *args)
{
  HkType types[] = { HK_TYPE_NIL, HK_TYPE_NUMBER, HK_TYPE_STRING };
  hk_state_check_argument_types(state, args, 1, 3, types);
  hk_return_if_not_ok(state);
  HkValue val = args[1];
  BigInt *result = bigint_new();
  if (hk_is_number(val))
  {
    hk_state_check_argument_int(state, args, 1);
    hk_return_if_not_ok(state);
    mpz_set_ui(result->num, (int64_t) hk_as_number(val));
    hk_state_push_userdata(state, (HkUserdata *) result);
    return;
  }
  if (hk_is_string(val))
  {
    int rc = mpz_set_str(result->num, hk_as_string(val)->chars, 10);
    if (rc)
    {
      mpz_clear(result->num);
      hk_state_push_nil(state);
      return;
    }
  }
  hk_state_push_userdata(state, (HkUserdata *) result);
}

static void from_string_call(HkState *state, HkValue *args)
{
  hk_state_check_argument_string(state, args, 1);
  hk_return_if_not_ok(state);
  HkString *str = hk_as_string(args[1]);
  HkValue val = args[2];
  int base = 10;
  if (!hk_is_nil(val))
  {
    hk_state_check_argument_int(state, args, 2);
    hk_return_if_not_ok(state);
    base = (int) hk_as_number(val);
  }
  BigInt *result = bigint_new();
  int rc = mpz_set_str(result->num, str->chars, base);
  if (rc)
  {
    mpz_clear(result->num);
    hk_state_push_nil(state);
    return;
  }
  hk_state_push_userdata(state, (HkUserdata *) result);
}

static void to_string_call(HkState *state, HkValue *args)
{
  hk_state_check_argument_userdata(state, args, 1);
  hk_return_if_not_ok(state);
  BigInt *bigint = (BigInt *) hk_as_userdata(args[1]);
  HkValue val = args[2];
  int base = 10;
  if (!hk_is_nil(val))
  {
    hk_state_check_argument_int(state, args, 2);
    hk_return_if_not_ok(state);
    base = (int) hk_as_number(val);
  }
  char *chars = mpz_get_str(NULL, base, bigint->num);
  HkString *str = hk_string_from_chars(-1, chars);
  free(chars);
  hk_state_push_string(state, str);
}

static void from_bytes_call(HkState *state, HkValue *args)
{
  hk_state_check_argument_string(state, args, 1);
  hk_return_if_not_ok(state);
  HkString *str = hk_as_string(args[1]);
  BigInt *result = bigint_new();
  mpz_import(result->num, str->length, 1, 1, 0, 0, str->chars);
  hk_state_push_userdata(state, (HkUserdata *) result);
}

static void to_bytes_call(HkState *state, HkValue *args)
{
  hk_state_check_argument_userdata(state, args, 1);
  hk_return_if_not_ok(state);
  BigInt *bigint = (BigInt *) hk_as_userdata(args[1]);
  size_t length;
  char *chars = mpz_export(NULL, &length, 1, 1, 0, 0, bigint->num);
  HkString *str = hk_string_from_chars((int) length, chars);
  free(chars);
  hk_state_push_string(state, str);
}

static void sign_call(HkState *state, HkValue *args)
{
  hk_state_check_argument_userdata(state, args, 1);
  hk_return_if_not_ok(state);
  BigInt *bigint = (BigInt *) hk_as_userdata(args[1]);
  int sign = mpz_sgn(bigint->num);
  hk_state_push_number(state, sign);
}

static void add_call(HkState *state, HkValue *args)
{
  hk_state_check_argument_userdata(state, args, 1);
  hk_return_if_not_ok(state);
  HkValue val1 = args[1];
  HkValue val2 = args[2];
  BigInt *bigint1 = (BigInt *) hk_as_userdata(val1);
  if (hk_is_int(val2))
  {
    BigInt *result = bigint_new();
    mpz_add_ui(result->num, bigint1->num, (int64_t) hk_as_number(val2));
    hk_state_push_userdata(state, (HkUserdata *) result);
    return;
  }
  hk_state_check_argument_userdata(state, args, 2);
  hk_return_if_not_ok(state);
  BigInt *bigint2 = (BigInt *) hk_as_userdata(val2);
  BigInt *result = bigint_new();
  mpz_add(result->num, bigint1->num, bigint2->num);
  hk_state_push_userdata(state, (HkUserdata *) result);
}

static void sub_call(HkState *state, HkValue *args)
{
  hk_state_check_argument_userdata(state, args, 1);
  hk_return_if_not_ok(state);
  HkValue val1 = args[1];
  HkValue val2 = args[2];
  BigInt *bigint1 = (BigInt *) hk_as_userdata(val1);
  if (hk_is_int(val2))
  {
    BigInt *result = bigint_new();
    mpz_sub_ui(result->num, bigint1->num, (int64_t) hk_as_number(val2));
    hk_state_push_userdata(state, (HkUserdata *) result);
    return;
  }
  hk_state_check_argument_userdata(state, args, 2);
  hk_return_if_not_ok(state);
  BigInt *bigint2 = (BigInt *) hk_as_userdata(val2);
  BigInt *result = bigint_new();
  mpz_sub(result->num, bigint1->num, bigint2->num);
  hk_state_push_userdata(state, (HkUserdata *) result);
}

static void mul_call(HkState *state, HkValue *args)
{
  hk_state_check_argument_userdata(state, args, 1);
  hk_return_if_not_ok(state);
  HkValue val1 = args[1];
  HkValue val2 = args[2];
  BigInt *bigint1 = (BigInt *) hk_as_userdata(val1);
  if (hk_is_int(val2))
  {
    BigInt *result = bigint_new();
    mpz_mul_ui(result->num, bigint1->num, (int64_t) hk_as_number(val2));
    hk_state_push_userdata(state, (HkUserdata *) result);
    return;
  }
  hk_state_check_argument_userdata(state, args, 2);
  hk_return_if_not_ok(state);
  BigInt *bigint2 = (BigInt *) hk_as_userdata(val2);
  BigInt *result = bigint_new();
  mpz_mul(result->num, bigint1->num, bigint2->num);
  hk_state_push_userdata(state, (HkUserdata *) result);
}

static void div_call(HkState *state, HkValue *args)
{
  hk_state_check_argument_userdata(state, args, 1);
  hk_return_if_not_ok(state);
  HkValue val1 = args[1];
  HkValue val2 = args[2];
  BigInt *bigint1 = (BigInt *) hk_as_userdata(val1);
  if (hk_is_int(val2))
  {
    BigInt *result = bigint_new();
    mpz_div_ui(result->num, bigint1->num, (int64_t) hk_as_number(val2));
    hk_state_push_userdata(state, (HkUserdata *) result);
    return;
  }
  hk_state_check_argument_userdata(state, args, 2);
  hk_return_if_not_ok(state);
  BigInt *bigint2 = (BigInt *) hk_as_userdata(val2);
  BigInt *result = bigint_new();
  mpz_div(result->num, bigint1->num, bigint2->num);
  hk_state_push_userdata(state, (HkUserdata *) result);
}

static void mod_call(HkState *state, HkValue *args)
{
  hk_state_check_argument_userdata(state, args, 1);
  hk_return_if_not_ok(state);
  HkValue val1 = args[1];
  HkValue val2 = args[2];
  BigInt *bigint1 = (BigInt *) hk_as_userdata(val1);
  if (hk_is_int(val2))
  {
    BigInt *result = bigint_new();
    mpz_mod_ui(result->num, bigint1->num, (int64_t) hk_as_number(val2));
    hk_state_push_userdata(state, (HkUserdata *) result);
    return;
  }
  hk_state_check_argument_userdata(state, args, 2);
  hk_return_if_not_ok(state);
  BigInt *bigint2 = (BigInt *) hk_as_userdata(val2);
  BigInt *result = bigint_new();
  mpz_mod(result->num, bigint1->num, bigint2->num);
  hk_state_push_userdata(state, (HkUserdata *) result);
}

static void pow_call(HkState *state, HkValue *args)
{
  hk_state_check_argument_userdata(state, args, 1);
  hk_return_if_not_ok(state);
  HkValue val1 = args[1];
  HkValue val2 = args[2];
  BigInt *bigint1 = (BigInt *) hk_as_userdata(val1);
  if (hk_is_int(val2))
  {
    BigInt *result = bigint_new();
    mpz_pow_ui(result->num, bigint1->num, (int64_t) hk_as_number(val2));
    hk_state_push_userdata(state, (HkUserdata *) result);
    return;
  }
  hk_state_check_argument_userdata(state, args, 2);
  hk_return_if_not_ok(state);
  BigInt *bigint2 = (BigInt *) hk_as_userdata(val2);
  BigInt *result = bigint_new();
  mpz_powm(result->num, bigint1->num, bigint2->num, bigint2->num);
  hk_state_push_userdata(state, (HkUserdata *) result);
}

static void powm_call(HkState *state, HkValue *args)
{
  hk_state_check_argument_userdata(state, args, 1);
  hk_return_if_not_ok(state);
  HkValue val1 = args[1];
  HkValue val2 = args[2];
  HkValue val3 = args[3];
  BigInt *bigint1 = (BigInt *) hk_as_userdata(val1);
  if (hk_is_int(val2) && hk_is_int(val3))
  {
    mpz_t num2;
    mpz_init_set_ui(num2, (int64_t) hk_as_number(val2));
    mpz_t num3;
    mpz_init_set_ui(num3, (int64_t) hk_as_number(val3));
    BigInt *result = bigint_new();
    mpz_powm(result->num, bigint1->num, num2, num3);
    mpz_clear(num2);
    mpz_clear(num3);
    hk_state_push_userdata(state, (HkUserdata *) result);
    return;
  }
  if (hk_is_int(val2) && hk_is_userdata(val3))
  {
    mpz_t num2;
    mpz_init_set_ui(num2, (int64_t) hk_as_number(val2));
    BigInt *bigint3 = (BigInt *) hk_as_userdata(val3);
    BigInt *result = bigint_new();
    mpz_powm(result->num, bigint1->num, num2, bigint3->num);
    mpz_clear(num2);
    hk_state_push_userdata(state, (HkUserdata *) result);
    return;
  }
  if (hk_is_userdata(val2) && hk_is_int(val3))
  {
    BigInt *bigint2 = (BigInt *) hk_as_userdata(val2);
    mpz_t num3;
    mpz_init_set_ui(num3, (int64_t) hk_as_number(val3));    
    BigInt *result = bigint_new();
    mpz_powm(result->num, bigint1->num, bigint2->num, num3);
    mpz_clear(num3);
    hk_state_push_userdata(state, (HkUserdata *) result);
    return;
  }
  hk_state_check_argument_userdata(state, args, 2);
  hk_return_if_not_ok(state);
  hk_state_check_argument_userdata(state, args, 3);
  hk_return_if_not_ok(state);
  BigInt *bigint2 = (BigInt *) hk_as_userdata(val2);
  BigInt *bigint3 = (BigInt *) hk_as_userdata(val3);
  BigInt *result = bigint_new();
  mpz_powm(result->num, bigint1->num, bigint2->num, bigint3->num);
  hk_state_push_userdata(state, (HkUserdata *) result);
}

static void sqrt_call(HkState *state, HkValue *args)
{
  hk_state_check_argument_userdata(state, args, 1);
  hk_return_if_not_ok(state);
  BigInt *bigint = (BigInt *) hk_as_userdata(args[1]);
  BigInt *result = bigint_new();
  mpz_sqrt(result->num, bigint->num);
  hk_state_push_userdata(state, (HkUserdata *) result);
}

static void sqrtm_prime_call(HkState *state, HkValue *args)
{
  hk_state_check_argument_userdata(state, args, 1);
  hk_return_if_not_ok(state);
  HkValue val1 = args[1];
  HkValue val2 = args[2];
  BigInt *bigint1 = (BigInt *) hk_as_userdata(val1);
  if (hk_is_int(val2))
  {
    mpz_t num2;
    mpz_init_set_ui(num2, (int64_t) hk_as_number(val2));
    BigInt *result = bigint_new();
    sqrtm_prime(result->num, bigint1->num, num2);
    mpz_clear(num2);
    hk_state_push_userdata(state, (HkUserdata *) result);
    return;
  }
  hk_state_check_argument_userdata(state, args, 2);
  hk_return_if_not_ok(state);
  BigInt *bigint2 = (BigInt *) hk_as_userdata(val2);
  BigInt *result = bigint_new();
  sqrtm_prime(result->num, bigint1->num, bigint2->num);
  hk_state_push_userdata(state, (HkUserdata *) result);
}

static void neg_call(HkState *state, HkValue *args)
{
  hk_state_check_argument_userdata(state, args, 1);
  hk_return_if_not_ok(state);
  BigInt *bigint = (BigInt *) hk_as_userdata(args[1]);
  BigInt *result = bigint_new();
  mpz_neg(result->num, bigint->num);
  hk_state_push_userdata(state, (HkUserdata *) result);
}

static void abs_call(HkState *state, HkValue *args)
{
  hk_state_check_argument_userdata(state, args, 1);
  hk_return_if_not_ok(state);
  BigInt *bigint = (BigInt *) hk_as_userdata(args[1]);
  BigInt *result = bigint_new();
  mpz_abs(result->num, bigint->num);
  hk_state_push_userdata(state, (HkUserdata *) result);
}

static void compare_call(HkState *state, HkValue *args)
{
  hk_state_check_argument_userdata(state, args, 1);
  hk_return_if_not_ok(state);
  HkValue val1 = args[1];
  HkValue val2 = args[2];
  BigInt *bigint1 = (BigInt *) hk_as_userdata(val1);
  if (hk_is_int(val2))
  {
    int result = mpz_cmp_ui(bigint1->num, (int64_t) hk_as_number(val2));
    hk_state_push_number(state, result);
    return;
  }
  hk_state_check_argument_userdata(state, args, 2);
  hk_return_if_not_ok(state);
  BigInt *bigint2 = (BigInt *) hk_as_userdata(val2);
  int result = mpz_cmp(bigint1->num, bigint2->num);
  hk_state_push_number(state, result);
}

static void invertm_call(HkState *state, HkValue *args)
{
  hk_state_check_argument_userdata(state, args, 1);
  hk_return_if_not_ok(state);
  HkValue val1 = args[1];
  HkValue val2 = args[2];
  BigInt *bigint1 = (BigInt *) hk_as_userdata(val1);
  if (hk_is_int(val2))
  {
    mpz_t num2;
    mpz_init_set_ui(num2, (int64_t) hk_as_number(val2));
    BigInt *result = bigint_new();
    int rc = mpz_invert(result->num, bigint1->num, num2);
    if (!rc)
    {
      mpz_clear(num2);
      hk_state_push_nil(state);
      return;
    }
    mpz_clear(num2);
    hk_state_push_userdata(state, (HkUserdata *) result);
    return;
  }
  hk_state_check_argument_userdata(state, args, 2);
  hk_return_if_not_ok(state);
  BigInt *bigint2 = (BigInt *) hk_as_userdata(val2);
  BigInt *result = bigint_new();
  int rc = mpz_invert(result->num, bigint1->num, bigint2->num);
  if (!rc)
  {
    hk_state_push_nil(state);
    return;
  }
  hk_state_push_userdata(state, (HkUserdata *) result);
}

static void size_call(HkState *state, HkValue *args)
{
  hk_state_check_argument_userdata(state, args, 1);
  hk_return_if_not_ok(state);
  BigInt *bigint = (BigInt *) hk_as_userdata(args[1]);
  HkValue val = args[2];
  int base = 10;
  if (!hk_is_nil(val))
  {
    hk_state_check_argument_int(state, args, 2);
    hk_return_if_not_ok(state);
    base = (int) hk_as_number(val);
  }
  int result = mpz_sizeinbase(bigint->num, base);
  hk_state_push_number(state, result);
}

static void testbit_call(HkState *state, HkValue *args)
{
  hk_state_check_argument_userdata(state, args, 1);
  hk_return_if_not_ok(state);
  hk_state_check_argument_int(state, args, 2);
  hk_return_if_not_ok(state);
  BigInt *bigint = (BigInt *) hk_as_userdata(args[1]);
  int index = (int) hk_as_number(args[2]);
  int result = mpz_tstbit(bigint->num, index);
  hk_state_push_number(state, result);
}

HK_LOAD_MODULE_HANDLER(bigint)
{
  hk_state_push_string_from_chars(state, -1, "bigint");
  hk_return_if_not_ok(state);
  hk_state_push_string_from_chars(state, -1, "new");
  hk_return_if_not_ok(state);
  hk_state_push_new_native(state, "new", 2, new_call);
  hk_return_if_not_ok(state);
  hk_state_push_string_from_chars(state, -1, "from_string");
  hk_return_if_not_ok(state);
  hk_state_push_new_native(state, "from_string", 2, from_string_call);
  hk_return_if_not_ok(state);
  hk_state_push_string_from_chars(state, -1, "to_string");
  hk_return_if_not_ok(state);
  hk_state_push_new_native(state, "to_string", 2, to_string_call);
  hk_return_if_not_ok(state);
  hk_state_push_string_from_chars(state, -1, "from_bytes");
  hk_return_if_not_ok(state);
  hk_state_push_new_native(state, "from_bytes", 1, from_bytes_call);
  hk_return_if_not_ok(state);
  hk_state_push_string_from_chars(state, -1, "to_bytes");
  hk_return_if_not_ok(state);
  hk_state_push_new_native(state, "to_bytes", 1, to_bytes_call);
  hk_return_if_not_ok(state);
  hk_state_push_string_from_chars(state, -1, "sign");
  hk_return_if_not_ok(state);
  hk_state_push_new_native(state, "sign", 1, sign_call);
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
  hk_state_push_string_from_chars(state, -1, "powm");
  hk_return_if_not_ok(state);
  hk_state_push_new_native(state, "powm", 3, powm_call);
  hk_return_if_not_ok(state);
  hk_state_push_string_from_chars(state, -1, "sqrt");
  hk_return_if_not_ok(state);
  hk_state_push_new_native(state, "sqrt", 1, sqrt_call);
  hk_return_if_not_ok(state);
  hk_state_push_string_from_chars(state, -1, "sqrtm_prime");
  hk_return_if_not_ok(state);
  hk_state_push_new_native(state, "sqrtm_prime", 2, sqrtm_prime_call);
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
  hk_state_push_string_from_chars(state, -1, "invertm");
  hk_return_if_not_ok(state);
  hk_state_push_new_native(state, "invertm", 2, invertm_call);
  hk_return_if_not_ok(state);
  hk_state_push_string_from_chars(state, -1, "size");
  hk_return_if_not_ok(state);
  hk_state_push_new_native(state, "size", 2, size_call);
  hk_return_if_not_ok(state);
  hk_state_push_string_from_chars(state, -1, "testbit");
  hk_return_if_not_ok(state);
  hk_state_push_new_native(state, "testbit", 2, testbit_call);
  hk_return_if_not_ok(state);
  hk_state_construct(state, 21);
}
