/*
 * Copyright (c) 2016 Intel Corporation.
 *
 * SPDX-License-Identifier: Apache-2.0
 */


#include "test_fat.h"

void test_main(void)
{
	ztest_test_suite(fat_fs_basic_test,
			 ztest_unit_test(test_fat_mount),
			 ztest_unit_test(test_fat_file),
			 ztest_unit_test(test_fat_dir),
			 ztest_unit_test(test_fat_fs),
			 ztest_unit_test(test_fat_rename));
	ztest_run_test_suite(fat_fs_basic_test);
}
