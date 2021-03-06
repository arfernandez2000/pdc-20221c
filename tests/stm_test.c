#include <stdlib.h>
#include <stdbool.h>
#include <check.h>
#include "../selector/selector.h"
#include "stm.h"

enum test_states {
    A,
    B,
    C,
};

struct data {
    bool arrived  [3];
    bool departed[3];
    unsigned i;
};

static void
on_arrival(const unsigned state, selector_key *s_key) {
    struct data *d = (struct data *)s_key->data;
    d->arrived[state] = true;
}

static void
on_departure(const unsigned state,selector_key *s_key) {
    struct data *d = (struct data *)s_key->data;
    d->departed[state] = true;
}

static unsigned
on_read_ready(selector_key *s_key) {
    struct data *d = (struct data *)s_key->data;
    unsigned ret;

    if(d->i < C) {
        ret = ++d->i;
    } else {
        ret = C;
    }
    return ret;
}

static unsigned
on_write_ready(selector_key *s_key) {
    return on_read_ready(s_key);
}

static const struct state_definition statbl[] = {
    {
        .state          = A,
        .on_arrival     = on_arrival,
        .on_departure   = on_departure,
        .on_read_ready  = on_read_ready,
        .on_write_ready = on_write_ready,
    },{
        .state          = B,
        .on_arrival     = on_arrival,
        .on_departure   = on_departure,
        .on_read_ready  = on_read_ready,
        .on_write_ready = on_write_ready,
    },{
        .state          = C,
        .on_arrival     = on_arrival,
        .on_departure   = on_departure,
        .on_read_ready  = on_read_ready,
        .on_write_ready = on_write_ready,
    }
};

//static bool init = false;

START_TEST (test_buffer_misc) {
    state_machine stm = {
        .initial   = A,
        .max_state = C,
        .states    = statbl,
    };
    struct data data = {
        .i = 0,
    };
    selector_key  s_key = {
        .data = &data,
    };
    stm_init(&stm);
    ck_assert_uint_eq(A, stm_state(&stm));
    ck_assert_uint_eq(false,  data.arrived[A]);
    ck_assert_uint_eq(false,  data.arrived[B]);
    ck_assert_uint_eq(false,  data.arrived[C]);
    ck_assert_ptr_null(stm.current);

    stm_handler_read(&stm, &s_key);
    ck_assert_uint_eq(B,     stm_state(&stm));
    ck_assert_uint_eq(true,  data.arrived[A]);
    ck_assert_uint_eq(true,  data.arrived[B]);
    ck_assert_uint_eq(false, data.arrived[C]);
    ck_assert_uint_eq(true,  data.departed[A]);
    ck_assert_uint_eq(false, data.departed[B]);
    ck_assert_uint_eq(false, data.departed[C]);

    stm_handler_write(&stm, &s_key);
    ck_assert_uint_eq(C,     stm_state(&stm));
    ck_assert_uint_eq(true,  data.arrived[A]);
    ck_assert_uint_eq(true,  data.arrived[B]);
    ck_assert_uint_eq(true,  data.arrived[C]);
    ck_assert_uint_eq(true,  data.departed[A]);
    ck_assert_uint_eq(true,  data.departed[B]);
    ck_assert_uint_eq(false, data.departed[C]);

    stm_handler_read(&stm, &s_key);
    ck_assert_uint_eq(C,     stm_state(&stm));
    ck_assert_uint_eq(true,  data.arrived[A]);
    ck_assert_uint_eq(true,  data.arrived[B]);
    ck_assert_uint_eq(true,  data.arrived[C]);
    ck_assert_uint_eq(true,  data.departed[A]);
    ck_assert_uint_eq(true,  data.departed[B]);
    ck_assert_uint_eq(false, data.departed[C]);

    stm_handler_close(&stm, &s_key);
}
END_TEST

Suite *
suite(void) {
    Suite *s   = suite_create("nio_stm");
    TCase *tc  = tcase_create("nio_stm");

    tcase_add_test(tc, test_buffer_misc);
    suite_add_tcase(s, tc);

    return s;
}

int
main(void) {
    SRunner *sr  = srunner_create(suite());
    int number_failed;

    srunner_run_all(sr, CK_NORMAL);
    number_failed = srunner_ntests_failed(sr);
    srunner_free(sr);
    return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
