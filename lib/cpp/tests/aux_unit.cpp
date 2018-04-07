/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "memory.hpp"

#include <new>

struct mock_aux : ecl::aux
{
    mock_aux() :ecl::aux{} { }
    virtual void destroy();
};

// TODO: comment about why it is placed exacly here
void init_aux(void *in)
{
    new (in) mock_aux;
}

#include <CppUTest/TestHarness.h>
#include <CppUTest/CommandLineTestRunner.h>
#include <CppUTestExt/MockSupport.h>

void mock_aux::destroy()
{
    mock("aux").actualCall("destroy");
}

TEST_GROUP(aux)
{
    ecl::aux *test_aux;

    void setup()
    {
        void *ptr = aligned_alloc(alignof(mock_aux), sizeof(mock_aux));
        init_aux(ptr);
        test_aux = reinterpret_cast< mock_aux* >(ptr);
    }

    void teardown()
    {
        mock().clear();
    }
};

TEST(aux, constructed)
{
    auto weak = test_aux->weak_ref();
    auto ref = test_aux->ref();
    CHECK_EQUAL(0, weak);
    CHECK_EQUAL(0, ref);
}

TEST(aux, destroy_called_and_state_not_changed)
{
    mock("aux").expectOneCall("destroy");
    test_aux->destroy();

    auto weak = test_aux->weak_ref();
    auto ref = test_aux->ref();
    CHECK_EQUAL(0, weak);
    CHECK_EQUAL(0, ref);
}

TEST(aux, inc_dec_get_methods)
{
    test_aux->inc();
    test_aux->weak_inc();

    auto ref = test_aux->inc();
    auto weak = test_aux->weak_inc();

    CHECK_EQUAL(2, ref);
    CHECK_EQUAL(2, weak);

    ref = test_aux->ref();
    weak = test_aux->weak_ref();

    CHECK_EQUAL(2, ref);
    CHECK_EQUAL(2, weak);

    ref = test_aux->dec();
    weak = test_aux->weak_dec();

    CHECK_EQUAL(1, ref);
    CHECK_EQUAL(1, weak);
}

int main(int argc, char *argv[])
{
    return CommandLineTestRunner::RunAllTests(argc, argv);
}
