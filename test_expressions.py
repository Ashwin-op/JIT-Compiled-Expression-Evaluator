import subprocess
import sys
import unittest

from generate_expressions import generate_expressions


class TestExpressions(unittest.TestCase):
    def setUp(self):
        if subprocess.call(["make"]):
            self.fail("make failed")

    def tearDown(self):
        if subprocess.call(["make", "clean"]):
            self.fail("make clean failed")

    def test_valid_cases(self):
        for case in generate_expressions():
            print("Testing case: " + case)
            result = (
                subprocess.check_output(["./cs238", case])
                .decode(sys.stdout.encoding)
                .strip()
            )
            print("Got result: " + result)
            result = result.split("\n")[-1]
            result = float(result)
            assert abs(result - eval(case)) < 0.0001
            print("--------------------")


if __name__ == "__main__":
    unittest.main()
