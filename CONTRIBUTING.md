# Contribution guidelines

## Basic guidelines

### Use of generative AI is banned

Generative AI uses training data [based on plagiarism and piracy](https://web.archive.org/web/20250000000000*/https://www.theatlantic.com/technology/archive/2025/03/libgen-meta-openai/682093/), has [significant environmental costs associated with it](https://doi.org/10.21428/e4baedd9.9070dfe7), and [generates fundamentally insecure code](https://doi.org/10.1007/s10664-024-10590-1). GenAI is not ethically built, ethical to use, nor safe to use for programming applications. When caught, you will be permanently banned from contributing to the project, and any prior contributions will be checked and potentially reverted. 

### Testing policies

As a general rule, as much code as possible should be tested. If you add a new feature, please consider writing tests for the functionality as well. You can opt not to add tests, but doing so is strongly encouraged.

If you change functionality, existing tests that no longer work must be fixed.

### Git practices

This project does not follow any particular Git practices in branch names or commit messages. Try to make your commit messages useful, though. 

When creating a pull request, open it against the `master` branch. The `master` branch serves as the main development branch.

### Code style and naming conventions

* Naming conventions (C++):
    * Filenames, class names, struct names: PascalCase
    * Variable names, function names: camelCase
        * **Note:** Functions used for crowcpp endpoints have another special naming convention, in the form of `<http method><function name>`. The whole method is still camelCase. As an example, consider a `login` endpoint, which accepts `POST` requests. The backing method is called `postLogin`. For endpoints supporting multiple HTTP methods, use your best judgement, as this has not yet happened, so no conventions have been established.
    * Namespaces: Lower-case only, except for inner namespaces that act similarly to what purely static classes would in other languages, which use PascalCase.
    * Constants: SCREAMING_SNAKE_CASE
* Naming conventions (JavaScript):
    * Class names: PascalCase
    * Filenames: lowercase
    * Variable names, function names: camelCase
* Naming conventions (Python):
    * Filenames, variable names, function names: snake_case
    * Class names: CamelCase
    * Constants: SCREAMING_SNAKE_CASE

For the rest of the code style, try to match the rest of the code. No formatters are used because none of them are flexible enough to describe the code style I prefer in C++, and JS formatters require npm, and I don't want npm in this proejct. Do note that it's suggested to break lines around 120 characters at the latest, though going past that can be acceptable in certain circumstances. This does not apply to markdown and HTML

## Development setup

Before building and compiling, you need to install postgres. The actual setup of postgres is scripted for your convenience.

Building and setting up in commands:
```bash
git clone git@codeberg.org:LunarWatcher/jade
cd jade 

# First, install Conan. Using a venv is recommended:
python3 -m venv env
source env/bin/activate
pip3 install conan
# Optional, required to verify that the install is set up correctly. 
conan --help 
# For the foreseeable future, you have to manually add a different conan remote,
# because conan is ✨ special ✨
conan remote update conancenter --url https://center2.conan.io

# Builds must be done in a separate directory
mkdir build && cd build
# Additional optional flags you can use:
# -DJADE_SANITISE=ON/OFF
#   Enables -fsanitize. Default: OFF
# -DJADE_LINT=ON/OFF
#   Runs clang-tidy alongside the build. Requires clang-tidy installed. Default: OFF
# Note: if you have a sufficiently new CMake, you may also need -DCMAKE_POLICY_VERSION_MINIMUM=3.5
# because some dependencies are old.
cmake .. -DCMAKE_BUILD_TYPE=Debug 
make -j $(nproc)

# Set up the database. If you omit the PSQL_PASSWORD variable, you'll be
# interactively prompted for a password. SAVE THIS PASSWORD! You need it for 
# `config.json` in the next step
PSQL_PASSWORD="a password to use for the jade user in postgres" ../scripts/dev/dbinit.sh

# You need a config.json. After copying it, you MUST edit it
cp ../config.example.json ./config.json

# You can now run Jade:
make run
```

### File structure, and making changes

The project has four main directories:
* `src/`: Contains the backend code
* `www/`: Contains HTML, CSS, JS, and the mustache templates
* `tests/`: Contains the tests, and serves as the root for the C++ backend tests, along with certain standard files for use with the tests.
    * `tests/web`: Python-based frontend and integration tests

If you make changes to C++ code, you must recompile and restart. If you make changes to JS, CSS, or HTML/mustache templates, the changes are automatically picked up, and all you need to do is refresh. You may need to refresh without cache for certain files to update.

### Working with migrations

If you need to add or change the database, create a new migration file. These are special C++ files under `src/jade/db/migrations/`. You can look at `01Initial.hpp` for an example. Once you add your migration, add it to the list in `00Order.hpp`.

By default, the migrations run automatically when you start the server. However, if you need to experiment with the migrations and therefore need to revert, you can use `./bin/jade migrate down -1` to revert the last migration. An `up` command is also provided, but since the migrations are run automatically when you start the server, using it is not required. The suggested use for up and down are to rapidly test migrations. See `./bin/jade migrate --help` for additional commands and help.

If you need to purge the database, `./bin/jade migrate down` without any options rolls back all migrations, which is functionally equivalent to wiping the database. `scripts/dev/purge.sh` also offers this functionality, but with raw `psql` commands. 

### Running tests

There are two kinds of tests; frontend and backend tests. They're all run via CMake, with one of the following three targets:

* `test`: Runs backend tests
*`test-frontend`: Runs frontend and integration tests
* `test-all`: Runs both frontend and backend tests

> [!WARNING]
>
> Both test types have some special requirements. Failure to read this section in its entirety before running the tests **will** result in test failures.

#### Common requirements

One of the common requirements is access to a Postgres instance with a special database (`jadetest`). By default, the same user is used for both prod and tests. If you're comfortable creating these tables yourself, you're likely already setting up the database and a user. If you're not (or you're like me and can't be bothered writing out), you can run the following two scripts:
```
./scripts/dev/dbinit.sh
./scripts/dev/setup-testdb.sh
```

Note that the first script will not just create the user, but also create the `jade` database for running local debug instances. For the first script, you can also supply a `PSQL_PASSWORD` environment variable if you don't want to be interactively prompted for a password.

While the `PSQL_PASSWORD` variable is optional for the `dbinit` script, it is _required_ for both the frontend and backend tests. However, unlike for the `dbinit.sh` script, there's a conveniently provided `.env` file.

With the project folder as the working directory:
```
cp .env.template .env
```

Edit it, and populate the variables. Currently, only `PSQL_PASSWORD` is supported. Note that the tests assume the database is available at `localhost:5432`, and do not currently support configuring it to point elsewhere.

> [!WARNING]
>
> The `test-all` target has been specially defined to only run one test at a time. DO NOT run the frontend and backend tests concurrently. Part of the test setup includes several database wipes, which will result in frontend and backend tests colliding and erroneously failing.


#### Running backend tests

The backend tests have some of the fewest requirements. If you can build and run Jade, you can build and run the tests. There are no special requirements beyond the common requirements described in the previous section. You can just run
```
make test
```
(Or `make test-all`, but see "Running frontend tests" first)

and let it do its thing.

#### Running frontend tests

To run the frontend tests, you need:
* Python 3
* A working display of some kind. This does **not** need to be a real display if you're in a headless environment. Use [xvfb](https://wiki.archlinux.org/title/Xinit#Running_in_a_virtual_server) if you're on Xorg, and switch to xorg if you're on wayland[^6]. Xvfb is also useful regardless, as it lets you do anything else while the tests run. The frontend tests do take quite a bit longer to run than the backend tests due to browser and navigation overhead.
* Firefox and geckodriver; however, you need Linux to run any of this, so odds are good you have it already. Geckodriver comes presinstalled with firefox in most (?) cases, so on most consumer distros, you probably don't need to worry about this requirement.

Using `build/` as your working directory:
```bash
# Optional: create a venv
$ python3 -m venv env
$ source env/bin/activate
# Required:
# Note that users of ubuntu or derivatives need to pass --break-system-packages without a venv.
# Also note that doing so can break  your system packages, so be aware of the risks before you do this.
$ pip3 install -r ../tests/requirements.txt
$ make test-frontend
# Or, alternatively, to also run backend tests (please see "Running backend tests" first):
$ make test-all
```

**Note:** Several of the frontend tests require the `jade` executable. It's automatically built when you run `test-frontend` in CMake, and its location is automatically provided when the tests are run. 

>[!WARNING]
> 
> If you're using the Firefox snap (my sincerest condolences), Selenium will not cooperate. You will likely need to explicitly define the following environment variable:
> ```bash
> export SE_GECKODRIVER=/snap/bin/geckodriver
> ```
> 
> Otherwise, a number of different Selenium-related bugs will occur, that will cause the tests to erroneously fail. Some failure modes include:
> 
> * Timeouts during the test, specifically when creating the browser
> * Various errors ("binary is not a firefox executable", "the geckodriver detected in PATH may not be compatbile with the detected firefox version")
>   * The incompatible versions may appear regardless, because Ubuntu's snap is horseshit, out of date, and contain unsynced  geckodriver and firefox
> * Outright failure to find Firefox and/or Geckodriver
