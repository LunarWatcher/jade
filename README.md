# jade - an ebook reader

> [!WARNING]
>
> Jade is currently in development, and in so early development that the database will break regularly. If you plan to use it before it's stable-ish, note that you will have to purge the database or manually work it into a current state

## Why?

I need two things from an ebook reader:

1. The ability to store progression
2. Good metadata management 

The two major arbitrary format actors in this game are calibre + calibre-web, and Kavita. Kavita's metadata management is absolute horseshit, especially for PDFs, and blatantly ignores tagging capitalisation. Calibre itself is mostly good, but progression tracking is the responsibility of the frontend - and calibre-web drops the ball hard. Calibre-web also has hard limits on filesizes that require editing an undocumented python file that has changed since the problem was initially discussed. 

Many of the other major readers have far greater problems than these two, primarily in format support. Notably PDFs are very poorly supported 

I need a reader with both web- and filesystem-based uploads with metadata changed after the fact, that supports mostly whatever format I throw at it. This project is meant to fill that particular gap.

## Principles

* **Performance.** Several of the current ebook readers are built in wildly inefficient backend tech stacks. The intended use is not large-scale production use, so it doesn't need the corresponding enterprise-grade optimisations. It's designed to be hostable on a potato machine or a server operating multiple other servers where the resource constraints are functionally identical. Though certain components are less performant (particularly those operating on certain parts of media extraction), the main part of the 
* **Database-only metadata management.** This is primarily to avoid modifying the original files. Though it does make it slightly more involved to re-export data, since the metadata is stored separately from the ebooks, it's much easier to implement arbitrary format support. All that's needed is a reader, with no writing capabilities needed[^2].

### Caveats

**Note that this will not work well with existing ebook libraries**. Importing from other ebook systems is not a priority, which means existing metadata will be discarded. I currently only have somewhere around 30 ebooks stored, with most of the metadata irrepairably butchered by Kavita, to the point where starting over is significantly easier and orders of magnitude faster than trying to implement an importer. However, importing is on the list of features that can be implemented if there is interest for whatever reason. Going by the average interest in my projects, there won't be in the foreseeable future, so it isn't something I'm implementing early. Equivalently, exporting metadata back out is not a priority.

Additionally, this is primarily intended as an extended and user-syncable.

## Requirements
* C++20 compiler
* Linux
* OpenSSL
* libpq (Debian: `libpq-dev`)
* PostgreSQL (not necessarily on the same server)
* CMake 3.10 or newer
* calibre[^1], installed via your system package manager (See: https://github.com/kovidgoyal/calibre?tab=readme-ov-file#calibre-package-versions-in-various-repositories)

## Installation 
```
bash <(curl -L https://codeberg.org/LunarWatcher/jade/raw/branch/master/scripts/install.sh)
```


## Planned  features

### Reader

| Done | Description | Blocked? |
| ---- | ----------- | -------- |
| [ ] | Configurable FixedLayout columns | [Yes](https://github.com/johnfactotum/foliate-js/issues/66) |
| [ ] | Fully scrollable files | [Yes](https://github.com/johnfactotum/foliate-js/issues/66) |
| [ ] | Inline zoom | |
| [ ] | Search | | 


### Everything else

| Done | Description | Blocked? |
| --- | --- | --- |
| [x] | Text search | |
| [x] | Tag search | |
| [x] | Author search | |
| [ ] | Sequels and prequels | |

[^1]: Calibre is used to deal with cover image generation due to its wide format support. The only other viable alternatives is to write it from scratch, or use a headless browser with the same renderer used for the website, to render it to png, which has 500M-1G worth of overhead. Optimally, writing it from scratch in a separate non-browser library would be beneficial (and reusable elsewhere while potentially being more performant), but this would be a massive undertaking that I do not feel like getting into unless there's other people who can provide additional resources with writing it. Until this exceedingly unlikely event happens, calibre is used to simplify the process.
[^2]: In C++ in particular, it seems to be easier to find parsers than it is to find writers. Avoiding writes means significantly less implementation complexity.
