import os

from time import sleep
import unittest

from selenium.webdriver.common.by import By
from selenium.common.exceptions import NoSuchElementException
import web.utils.driver as di

class PaginatorTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        # Reusing the webdriver where possible saves around 2 seconds per test
        cls.driver = di.init_firefox()

    @classmethod
    def tearDownClass(cls):
        cls.driver.quit()

    def init_page(self, count: int, current_page: int):
        self.driver.get("file://" + os.path.join(
            os.getcwd(),
            "html/paginator.html"
        ))
        self.driver.execute_script(f"""
        const paginator = document.getElementById("paginator");
        paginator.setAttribute("data-pages", "{count}");
        paginator.setAttribute("data-current-page", "{current_page}");

        loadPaginator()
        """)

    def get_button_container(self):
        return self.driver.find_elements(
            By.CSS_SELECTOR,
            "#paginator > ul > li"
        )

    def check_indices(self, expected, container, active):
        del container[0]
        del container[-1]
        for i, expected_page in enumerate(expected):
            elem = container[i]
            if expected_page is None:
                with self.assertRaises(NoSuchElementException):
                    elem.find_element(By.TAG_NAME, "a")
            else:
                anchor = elem.find_element(By.TAG_NAME, "a")
                self.assertIsNotNone(anchor.get_attribute("href"))
                self.assertTrue(
                    f"?page={expected_page}" in anchor.get_attribute("href"),
                    "Found {}, expected page {}".format(
                        anchor.get_attribute("href"),
                        expected_page
                    )
                )
                if expected_page == active:
                    self.assertTrue(
                        "active" in anchor.get_attribute("class"),
                        "i = " + str(i)
                    )
                else:
                    self.assertFalse(
                        "active" in anchor.get_attribute("class"),
                        "i = " + str(i)
                    )


    def test_edges(self):
        self.init_page(4, 1)

        container = self.get_button_container()
        self.assertEqual(len(container), 6)
        self.check_indices([1, 2, 3, 4], container, 1)

        self.init_page(4, 4)
        container = self.get_button_container()
        self.assertEqual(len(container), 6)
        self.check_indices([1, 2, 3, 4], container, 4)

    def test_overflow(self):
        self.init_page(100, 50)

        container = self.get_button_container()
        # 2 nav buttons + 2 spacers + 7 page buttons = 11
        self.assertEqual(len(container), 11)
        self.check_indices(
            [1, None, 48, 49, 50, 51, 52, None, 100],
            container,
            50
        )

    def test_near_edge_overflow_upper(self):
        self.init_page(10, 7)

        container = self.get_button_container()
        # 2 nav buttons + 1 spacer + 7 page buttons = 11
        self.assertEqual(len(container), 10)
        self.check_indices(
            [1, None, 5, 6, 7, 8, 9, 10],
            container,
            7
        )

    def test_near_edge_overflow_lower(self):
        self.init_page(10, 4)

        container = self.get_button_container()
        self.assertEqual(len(container), 10)
        self.check_indices(
            [1, 2, 3, 4, 5, 6, None, 10],
            container,
            4
        )

    def test_disabled_when_minimal(self):
        self.init_page(1, 1)

        container = self.get_button_container()
        self.assertEqual(len(container), 3)
        self.assertIsNone(
            container[0]
                .find_element(By.TAG_NAME, "a")
                .get_attribute("href")
        )
        self.assertIsNone(
            container[-1]
                .find_element(By.TAG_NAME, "a")
                .get_attribute("href")
        )
        self.check_indices([1], container, 1)

    def test_disabled_upper_edges(self):
        self.init_page(100, 100)
        container = self.get_button_container()
        self.assertEqual(len(container), 10)
        self.assertIsNotNone(
            container[0]
                .find_element(By.TAG_NAME, "a")
                .get_attribute("href")
        )
        self.assertIsNone(
            container[-1]
                .find_element(By.TAG_NAME, "a")
                .get_attribute("href")
        )
        self.check_indices([1, None, 95, 96, 97, 98, 99, 100],
                           container, 100)

    def test_disabled_lower_edge(self):
        self.init_page(100, 1)
        container = self.get_button_container()
        self.assertEqual(len(container), 10)
        self.assertIsNone(
            container[0]
                .find_element(By.TAG_NAME, "a")
                .get_attribute("href")
        )
        self.assertIsNotNone(
            container[-1]
                .find_element(By.TAG_NAME, "a")
                .get_attribute("href")
        )
        self.check_indices([1, 2, 3, 4, 5, 6, None, 100],
                           container, 1)


    def test_invalid(self):
        self.init_page(5, 85643)
        self.assertEqual(
            self.driver.execute_script(
                "return document.getElementById('paginator')"
                ".style.visibility"
            ), "hidden"
        )
