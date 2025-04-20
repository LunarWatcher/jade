# jade - an ebook reader

## Why?

I need two things from an ebook reader:

1. The ability to store progression
2. Good metadata management 

The two major arbitrary format actors in this game are calibre + calibre-web, and Kavita. Kavita's metadata management is absolute horseshit, especially for PDFs, and blatantly ignores tagging capitalisation. Calibre itself is mostly good, but progression tracking is the responsibility of the frontend - and calibre-web drops the ball hard. Calibre-web also has hard limits on filesizes that require editing an undocumented python file that has changed since the problem was initially discussed. 

Many of the other major readers have far greater problems than these two, primarily in format support. Notably PDFs are very poorly supported 

I need a reader with both web- and filesystem-based uploads with metadata changed after the fact, that supports mostly whatever format I throw at it. This project is meant to fill that particular gap.

## Principles

* **Performance.** Several of the current ebook readers are built in wildly inefficient backend tech stacks. The intended use is not large-scale production use, so it doesn't need the corresponding enterprise-grade optimisations. It's designed to be hostable on a potato machine or a server operating multiple other servers where the resource constraints are functionally identical. Though certain components are less performant (particularly those operating on certain parts of media extraction), the main part of the 
* **Database-only metadata management.** This is primarily to avoid modifying the original files, but comes with the unintended consequence that existing metadata is ignored, with the intended consequence of allowing you to fully use your own system

### Caveats

**Note that this will not work well with existing ebook libraries**. Importing from other ebook systems is not a priority, which means existing metadata will be discarded. I currently only have somewhere around 30 ebooks stored, with most of the metadata irrepairably butchered by Kavita, to the point where starting over is significantly easier and orders of magnitude faster than trying to implement an importer. However, importing is on the list of features that can be implemented if there is interest for whatever reason. Going by the average interest in my projects, there won't be in the foreseeable future, so it isn't something I'm implementing early. 

Additionally, this is primarily intended as an extended and user-syncable.

## Requirements
* C++20 compiler
* Linux
* OpenSSL
* libpq (Debian: `libpq-dev`) and postgresql
* Python3 + `pip3 install calibre`[^1][^2]

For Debian-based systems, only `libpq-dev` and OpenSSL[^3] is required to be installed. If you're running the database on the same hardware as jade, `postgresql` is also needed.

[^1]: Can (and probably should) be a virtualenv to ensure a stable install coupled to jade. This is the preferred system for the installer script.
[^2]: Calibre is used to deal with cover image generation due to its wide format support. The only other viable alternatives is to write it from scratch, or use a headless browser with the same renderer used for the website, to render it to png, which has 500M-1G worth of overhead. Optimally, writing it from scratch in a separate library would be beneficial (and reusable elsewhere while potentially being more performant), but this would be a massive undertaking that I do not feel like getting into unless there's other people who can provide additional resources with writing it. Until this exceedingly unlikely event happens, calibre is used to simplify the process.
[^3]: Semi-often preinstalled with all required components
