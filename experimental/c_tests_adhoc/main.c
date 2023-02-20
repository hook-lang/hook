#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <assert.h>
#include <stdint.h>
#include "../../src/hk_scanner.h"
#include "hk_string.h"

const int NOT_EXITED_BY_THE_FUNCTION = -999;

fork_and_get_exit_value(void (*fn)())
{
    pid_t pid = fork();
    assert(pid >= 0);
    if (pid > 0)
    {
        int forked_status;
        waitpid(pid, &forked_status, 0);
        if (WIFEXITED(forked_status))
        {
            int forked_exit_value = WEXITSTATUS(forked_status);
            return forked_exit_value;
        }
    }
    if (pid == 0)
    {
        fn();
        exit(NOT_EXITED_BY_THE_FUNCTION);
    }
}

do_exit_on_invalid_raw_line_feed_inside_string()
{
    scanner_t scanner;
    hk_string_t *file = hk_string_from_chars(-1, "<terminal>");
    hk_string_t *source = hk_string_from_chars(-1, "\"te\nst\"");
    scanner_init(&scanner, file, source);
    hk_string_free(file);
    hk_string_free(source);
    hk_string_free(scanner.token.value);
}

test_scanner_exit_on_invalid_raw_line_feed_inside_string()
{
    int exit_value = fork_and_get_exit_value(
        do_exit_on_invalid_raw_line_feed_inside_string());
    assert(exit_value != NOT_EXITED_BY_THE_FUNCTION);
    assert(exit_value == EXIT_FAILURE);
}

test_scanner_match_string()
{
    scanner_t scanner;
    hk_string_t *file = hk_string_from_chars(-1, "<terminal>");
    hk_string_t *source = hk_string_from_chars(-1, "\"test\"");
    scanner_init(&scanner, file, source);
    assert(scanner.token.type == TOKEN_STRING);
    assert(scanner.token.line == 1);
    assert(scanner.token.col == 1);
    assert(scanner.token.length == 4);
    assert(scanner.token.start[0] == 't');
    assert(scanner.token.start[1] == 'e');
    assert(scanner.token.start[2] == 's');
    assert(scanner.token.start[3] == 't');
    hk_string_free(file);
    hk_string_free(source);
    hk_string_free(scanner.token.value);
}

test_scanner_match_string_with_new_line_escape_sequence()
{
    scanner_t scanner;
    hk_string_t *file = hk_string_from_chars(-1, "<terminal>");
    hk_string_t *source = hk_string_from_chars(-1, "\"te\\nst\"");
    scanner_init(&scanner, file, source);
    assert(scanner.token.type == TOKEN_STRING);
    assert(scanner.token.line == 1);
    assert(scanner.token.col == 1);
    assert(scanner.token.length == 6);
    assert(scanner.token.start[0] == 't');
    assert(scanner.token.start[1] == 'e');
    assert(scanner.token.start[2] == '\\');
    assert(scanner.token.start[3] == 'n');
    assert(scanner.token.start[4] == 's');
    assert(scanner.token.start[5] == 't');
    assert(scanner.token.value->length == 5);
    assert(scanner.token.value->chars[0] == 't');
    assert(scanner.token.value->chars[1] == 'e');
    assert(scanner.token.value->chars[2] == '\n');
    assert(scanner.token.value->chars[3] == 's');
    assert(scanner.token.value->chars[4] == 't');
    hk_string_free(file);
    hk_string_free(source);
    hk_string_free(scanner.token.value);
}

test_scanner_match_string_with_tab_escape_sequence()
{
    scanner_t scanner;
    hk_string_t *file = hk_string_from_chars(-1, "<terminal>");
    hk_string_t *source = hk_string_from_chars(-1, "\"test\\t\"");
    scanner_init(&scanner, file, source);
    assert(scanner.token.type == TOKEN_STRING);
    assert(scanner.token.line == 1);
    assert(scanner.token.col == 1);
    assert(scanner.token.length == 6);
    assert(scanner.token.start[0] == 't');
    assert(scanner.token.start[1] == 'e');
    assert(scanner.token.start[2] == 's');
    assert(scanner.token.start[3] == 't');
    assert(scanner.token.start[4] == '\\');
    assert(scanner.token.start[5] == 't');
    assert(scanner.token.value->length == 5);
    assert(scanner.token.value->chars[0] == 't');
    assert(scanner.token.value->chars[1] == 'e');
    assert(scanner.token.value->chars[2] == 's');
    assert(scanner.token.value->chars[3] == 't');
    assert(scanner.token.value->chars[4] == '\t');
    hk_string_free(file);
    hk_string_free(source);
    hk_string_free(scanner.token.value);
}

test_scanner_match_string_with_multiple_escape_sequence()
{
    scanner_t scanner;
    hk_string_t *file = hk_string_from_chars(-1, "<terminal>");
    hk_string_t *source = hk_string_from_chars(-1, "\"\\tte\\r\\nst\"");
    scanner_init(&scanner, file, source);
    assert(scanner.token.type == TOKEN_STRING);
    assert(scanner.token.line == 1);
    assert(scanner.token.col == 1);
    assert(scanner.token.length == 10);
    assert(scanner.token.start[0] == '\\');
    assert(scanner.token.start[1] == 't');
    assert(scanner.token.start[2] == 't');
    assert(scanner.token.start[3] == 'e');
    assert(scanner.token.start[4] == '\\');
    assert(scanner.token.start[5] == 'r');
    assert(scanner.token.start[6] == '\\');
    assert(scanner.token.start[7] == 'n');
    assert(scanner.token.start[8] == 's');
    assert(scanner.token.start[9] == 't');
    assert(scanner.token.value->length == 7);
    assert(scanner.token.value->chars[0] == '\t');
    assert(scanner.token.value->chars[1] == 't');
    assert(scanner.token.value->chars[2] == 'e');
    assert(scanner.token.value->chars[3] == '\r');
    assert(scanner.token.value->chars[4] == '\n');
    assert(scanner.token.value->chars[5] == 's');
    assert(scanner.token.value->chars[6] == 't');
    hk_string_free(file);
    hk_string_free(source);
    hk_string_free(scanner.token.value);
}

test_concat_chars_to_empty_string_until_the_capacity_doubles()
{
    hk_string_t *str = hk_string_new();
    hk_string_inplace_concat_char(str, 'a');
    assert(HK_STRING_MIN_CAPACITY == 8);
    assert(str->capacity == HK_STRING_MIN_CAPACITY);
    assert(str->length == 1);
    assert(str->chars[0] == 'a');
    assert(str->chars[1] == '\0');
    hk_string_inplace_concat_char(str, 'b');
    assert(str->capacity == HK_STRING_MIN_CAPACITY);
    assert(str->length == 2);
    assert(str->chars[0] == 'a');
    assert(str->chars[1] == 'b');
    assert(str->chars[2] == '\0');
    hk_string_inplace_concat_char(str, 'c');
    assert(str->capacity == HK_STRING_MIN_CAPACITY);
    assert(str->length == 3);
    assert(str->chars[0] == 'a');
    assert(str->chars[1] == 'b');
    assert(str->chars[2] == 'c');
    assert(str->chars[3] == '\0');
    hk_string_inplace_concat_char(str, 'd');
    assert(str->capacity == HK_STRING_MIN_CAPACITY);
    assert(str->length == 4);
    assert(str->chars[0] == 'a');
    assert(str->chars[1] == 'b');
    assert(str->chars[2] == 'c');
    assert(str->chars[3] == 'd');
    assert(str->chars[4] == '\0');
    hk_string_inplace_concat_char(str, 'e');
    assert(str->capacity == HK_STRING_MIN_CAPACITY);
    assert(str->length == 5);
    assert(str->chars[0] == 'a');
    assert(str->chars[1] == 'b');
    assert(str->chars[2] == 'c');
    assert(str->chars[3] == 'd');
    assert(str->chars[4] == 'e');
    assert(str->chars[5] == '\0');
    hk_string_inplace_concat_char(str, 'f');
    assert(str->capacity == HK_STRING_MIN_CAPACITY);
    assert(str->length == 6);
    assert(str->chars[0] == 'a');
    assert(str->chars[1] == 'b');
    assert(str->chars[2] == 'c');
    assert(str->chars[3] == 'd');
    assert(str->chars[4] == 'e');
    assert(str->chars[5] == 'f');
    assert(str->chars[6] == '\0');
    hk_string_inplace_concat_char(str, 'g');
    assert(str->capacity == HK_STRING_MIN_CAPACITY);
    assert(str->length == 7);
    assert(str->chars[0] == 'a');
    assert(str->chars[1] == 'b');
    assert(str->chars[2] == 'c');
    assert(str->chars[3] == 'd');
    assert(str->chars[4] == 'e');
    assert(str->chars[5] == 'f');
    assert(str->chars[6] == 'g');
    assert(str->chars[7] == '\0');
    hk_string_inplace_concat_char(str, 'h');
    assert(str->capacity == HK_STRING_MIN_CAPACITY * 2);
    assert(str->length == 8);
    assert(str->chars[0] == 'a');
    assert(str->chars[1] == 'b');
    assert(str->chars[2] == 'c');
    assert(str->chars[3] == 'd');
    assert(str->chars[4] == 'e');
    assert(str->chars[5] == 'f');
    assert(str->chars[6] == 'g');
    assert(str->chars[7] == 'h');
    assert(str->chars[8] == '\0');
    hk_string_inplace_concat_char(str, 'i');
    assert(str->capacity == HK_STRING_MIN_CAPACITY * 2);
    assert(str->length == 9);
    assert(str->chars[0] == 'a');
    assert(str->chars[1] == 'b');
    assert(str->chars[2] == 'c');
    assert(str->chars[3] == 'd');
    assert(str->chars[4] == 'e');
    assert(str->chars[5] == 'f');
    assert(str->chars[6] == 'g');
    assert(str->chars[7] == 'h');
    assert(str->chars[8] == 'i');
    assert(str->chars[9] == '\0');
    hk_string_free(str);
}

test_create_empty_string()
{
    hk_string_t *str = hk_string_new();
    assert(str->capacity == HK_STRING_MIN_CAPACITY);
    assert(str->length == 0);
    assert(str->chars[0] == '\0');
    hk_string_free(str);
}

int32_t main(int32_t argc, const char **argv)
{
    test_create_empty_string();
    test_concat_chars_to_empty_string_until_the_capacity_doubles();
    test_scanner_match_string();
    test_scanner_match_string_with_new_line_escape_sequence();
    test_scanner_match_string_with_tab_escape_sequence();
    test_scanner_match_string_with_multiple_escape_sequence();
    test_scanner_exit_on_invalid_raw_line_feed_inside_string();
    return 0;
}