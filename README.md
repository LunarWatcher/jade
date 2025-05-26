# jade - an ebook reader

## Why?

I need two things from an ebook reader:

1. The ability to store progression
2. Good metadata management for things that matter

The two major arbitrary format actors in this game are calibre + calibre-web, and Kavita. Kavita's metadata management is absolute horseshit, especially for PDFs, and blatantly ignores tagging capitalisation. Calibre itself is mostly good, but progression tracking is the responsibility of the frontend - and calibre-web drops the ball hard. Calibre-web also has hard limits on filesizes that require editing an undocumented python file that has changed since the problem was initially discussed. 

Many of the other major readers have far greater problems than these two, primarily in format support. Notably PDFs are very poorly supported in some of them, with several potential ebook systems just outright not supporting them.

I need a reader with both web- and filesystem-based uploads with metadata changed after the fact, and that supports mostly whatever format I throw at it among the most common formats (in order: PDFs, followed by epubs far behind, and much further down the list, comic book archives). This project is meant to fill that particular gap.

## Principles

* **Performance.** Several of the current ebook readers are built in wildly inefficient backend tech stacks. The intended use for Jade is not large-scale production use with hundreds of thousands of users, so it doesn't need the corresponding enterprise-grade optimisations. It's designed to be hostable on a potato machine[^4] or a server operating multiple other servers where the resource constraints are functionally identical. Though certain components are less performant (particularly those operating on certain parts of media extraction), the main part of the system is designed to be light-weight to run, especially while idle, and not to compete with megacorp servers in requests per second.
* **Database-only metadata management.** This is primarily to avoid modifying the original files. Though it does make it slightly more involved to re-export data, since the metadata is stored separately from the ebooks, it's much easier to implement arbitrary format and metadata support in a way that's flexible across all types. All that's needed is a reader, with no writing capabilities needed[^2].
    * This also means PDFs are not a second-class citizen; no reduced support just because PDFs don't support embedding as much metadata as epubs, nor because .epubs were implemented first and PDFs were shoehorned in afterwards. My library is overwhelmingly PDF-based, so this matters to me.

### Caveats

**Note that this will not work well with existing ebook libraries**. Importing from other ebook systems is not a priority, which means existing metadata will be discarded. I currently only have somewhere around 30 ebooks stored, with most of the metadata irrepairably butchered by Kavita, to the point where starting over is significantly easier and orders of magnitude faster than trying to implement an importer. However, importing is on the list of features that can be implemented if there is interest for whatever reason. Going by the average interest in my projects, there won't be in the foreseeable future, so it isn't something I'm implementing early. Equivalently, exporting metadata back out is not a priority. If you're interested in this functionality, you can add it yourself and open a pull request.



## Requirements
* C++20 compiler
* Linux
* OpenSSL
* libpq, zlib (Debian: `libpq-dev zlib1g`)
* PostgreSQL (not necessarily on the same server)
* CMake 3.28 or newer
* calibre[^1], installed via your system package manager (See: https://github.com/kovidgoyal/calibre?tab=readme-ov-file#calibre-package-versions-in-various-repositories)


## Installation 
> [!WARNING]
>
> During the installation process, you'll be prompted for a password for a new PostgreSQL user. You need to write this down, as it will not be automatically copied into the config file. If you lose the password, you need to [set a new one on your own](https://stackoverflow.com/a/28643961).

> [!WARNING]
>
> Do not follow the installation process for updates. Use `/opt/jade/scripts/update.sh` to update. Some versions may require additional post-update work, but any such changes will be announced in the updates section.

There's a convenient installation script that wraps everything aside installing dependencies:
```
JADE_DOMAIN=example.com JADE_CERT=/path/to/cert JADE_CERT_KEY=/path/to_key bash <(curl -L https://codeberg.org/LunarWatcher/jade/raw/branch/master/scripts/install.sh)
```

Supported variables for this script:
* `JADE_DOMAIN`: Required, sets `server_name` in nginx to `ebooks.${JADE_DOMAIN}`[^3]
* `JADE_CERT`/`JADE_CERT_KEY`: Sets nginx's `ssl_certificate` and `ssl_certificate_key` respectively[^3].
* `PSQL_PASSWORD`: Optional, used to set the password in postgres. If not provided, you'll be prompted for a password
* `JADE_USER`: Optional, defaults to `jade`


### Post-install steps

1. Edit `/etc/jade/config.json`
    * During the install process, you'll be asked to input a PSQL password. This is the file the password goes into
2. Edit `/etc/nginx/conf.d/jade.conf`, especially if you omitted any of the variables. The things you need to fix default to `FIXME`.
3. `sudo systemctl enable jade && sudo systemctl start jade && journalctl -ru jade`. You should not see any errors in the output. A warning about there being no libraries is expected at this point.
4. As long as the Jade service is running, `sudo systemctl restart nginx`. If you don't use nginx, now would be a good time to set up your reverse proxy.

Once the system is up and running, you should be able to connect to it. Make sure to create a user immediately - the first created user is automatically an administrator. Once you've done that, read `docs/README.md` for further setup steps, usage instructions, and answers to some common usage questions.

### FAQ

#### What if I don't use a reverse proxy?

Jade is currently explicitly designed for use with one. They're not that difficult to set up or scary to learn.

#### What if I don't use nginx?

Jade doesn't explicitly need nginx to work, but no config files for non-nginx reverse proxies are provided. You'll have to set it up yourself, and this is left as an exercise to the reader. However, due to how the installer is currently set up, it may fail when copying the nginx file, and may therefore require `/etc/nginx/conf.d/` to exist. This will be fixed in the future.

#### Docker?

No, and Docker won't be supported either. You're welcome to make your own Dockerfile, but you're entirely on your own if you do use Docker. A lot of the setup script can probably be joinked from `install.sh` if this is a path you want to go down.

## Updating Jade

Run `/opt/jade/scripts/update.sh`. The script takes care of the rest. If you used a non-standard `JADE_USER` During installation, you need to set the `JADE_USER` environment variable with the correct username.

## Development setup

TODO

### Running tests

There are two kinds of tests; frontend and backend tests. They're all run via CMake, with one of the following three targets:

* `test`: Runs backend tests
* `test-frontend`: Runs frontend and integration tests
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

>[!WARNING] For Ubuntu users
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

## Planned  features

Note that none of these lists are in any form of prioritised order. They're ordered by when they were added, not necessarily by when they're planned.

### Reader

| Done | Description | Blocked? |
| ---- | ----------- | -------- |
| [ ] | Configurable FixedLayout columns | [Yes](https://github.com/johnfactotum/foliate-js/issues/66) |
| [ ] | Fully scrollable files | [Yes](https://github.com/johnfactotum/foliate-js/issues/66) |
| [ ] | Inline zoom | |
| [ ] | Search | | 
| [ ] | Progress tracking or something | |
| [ ] | Improve the UX | |

### Everything else

| Done | Description | Blocked? |
| --- | --- | --- |
| [x] | Text search | |
| [x] | Tag search | |
| [x] | Author search | |
| [ ] | Sequels and prequels | |
| [ ] | System tests | Sort of, some of the extended tests require a browser, and I do not know if I have access to a webdriver in C++. Might need to mix in some Python + Selenium for some of the tests. |
| [x] | GH Actions[^5] | |
| [ ] | User settings | |
| [ ] | Deleting libraries | |
| [ ] | Age ratings, extended access controls, and extended views | |
| [ ] | A front page that isn't blank | |
| [ ] | Pagination + search retention bug | |

### Side projects for Later:tm:

* [ ] Port sonic and/or sonic's rust client to C++ via Rust bindings. Could implement the client from scratch, but I'd rather not. Sonic itself looks embeddable.

[^1]: Calibre is used to deal with cover image generation due to its wide format support. The only other viable alternatives is to write it from scratch, or use a headless browser with the same renderer used for the website, to render it to png, which has 500M-1G worth of overhead. Optimally, writing it from scratch in a separate non-browser library would be beneficial (and reusable elsewhere while potentially being more performant), but this would be a massive undertaking that I do not feel like getting into unless there's other people who can provide additional resources with writing it. Until this exceedingly unlikely event happens, calibre is used to simplify the process.
[^2]: In C++ in particular, it seems to be easier to find parsers than it is to find writers. Avoiding writes means significantly less implementation complexity.
[^3]: If you don't want SSL, this must be removed from `/etc/nginx/conf.d/jade.conf` after the fact. Other changes to nginx must be made manually in this file as well, for example if you don't want `ebooks.your-domain`
[^4]: In an early version, Jade used a whole 17MB of RAM while idle. Kavita immediately after installation uses nearly 400MB, and around 800MB in a prod-ish environment on my server. RAM does increase when loading books, but this is inevitable in both cases. Currently, Jade appears to be streaming the books, so even loading a 330MB PDF didn't bring Jade over 20MB.
[^5]: Funnily enough, even though this is a project mainly hosted on Codeberg, I plan to use GitHub Actions to avoid using Codeberg's resources.
[^6]: Wayland, at the time of writing, does not seem to have a viable alternative to Xvfb. XWayland may allow Xvfb to continue working, but native Wayland does not have a single good alternative that I can just point to and call it a day. If you prever Wayland so much you're using it in a headless environment, you can probably figure this out on your own. If you can't, switch back to Xorg while Wayland figures out its shit.
