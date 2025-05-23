import unittest
import web.utils.defaults
from web.utils.server import Server

class TestAuth(unittest.TestCase):
    def setUp(self):
        self.server = Server()

    def tearDown(self):
        self.server.stop()

    def test_admin(self):
        pass
