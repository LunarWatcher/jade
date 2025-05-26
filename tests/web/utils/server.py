import json
import logging
import os
import subprocess

from web.constants import PORT

logger = logging.getLogger(__name__)

class Server:
    def __init__(self,
        purge: bool = True,
        config_dir = "./config/default"
    ):
        self.load_config()
        self.server_loc = os.environ["SERVER_LOC"]
        self.config_dir = config_dir
        if self.server_loc is None:
            raise RuntimeError(
                "Misconfigured test environment: missing SERVER_LOC. "
                "You are probably not running from CMake when you should be"
            )

        if not os.getcwd().endswith("/tests"):
            raise RuntimeError("Misconfigured test environment: getcwd is "
                               "likely not /tests. You're probably not using "
                               "CMake when you should be")
        self.start(purge)

    def load_config(self):
        self.config = {}
        logger.info("Loading ../.env")
        with open("../.env") as f:
            for line in f.readlines():
                if (line.startswith("#")):
                    continue
                elif ("=" not in line):
                    continue

                (key, val) = line.split("=", 1)

                self.config[key] = val
        logger.info("../.env successfully loaded.")

    def start(self, purge: bool = True):
        self.server = subprocess.Popen(
            [
                self.server_loc,
                "--purge",
                "true" if purge else "false",
                "--override-config",
                self.config_dir
            ]
        )

    def stop(self):
        if self.server is None:
            raise RuntimeError("The server has either been stopped, "
                               "or it never ran in the first place")

        self.server.terminate()
        self.server.wait(10)

    def check_config_dir_inited(self):
        if not os.path.exists(self.config_dir):
            logger.info("%s not initialised yet", self.config_dir)

            os.makedirs(self.config_dir)
            with open("../config.example.json") as src:
                j = json.load(src)

                j["dbName"] = "jadetest";
                j["dbPassword"] = self.config["PSQL_PASSWORD"]
                j["port"] = PORT
                j["thumbnailCacheDir"] = "/"
                if j["dbPassword"] is None:
                    raise RuntimeError("Missing PSQL_PASSWORD environment variable")

                with open(os.path.join(self.config_dir,
                                       "config.json")) as cfg:
                    json.dump(j, cfg)
