# Terminology and categorisation

## User-defined categorisation

Jade has one primary forms of software categorisation; tags.

In addition, there's libraries, which is where the books are sourced from in the filesystem. Libraries do not serve categorisation in the same way when viewing it, at least not for now, and the plan is for them to mainly serve automatic access control and certain kinds of automatic categorisation down the line. The main forms of searchable and editable categorisation are the software categorisation types.

### Tags

Tags as a concept are exactly what you'd expect them to be. Arbitrary words classify content, based on whatever you want to classify it as. Tags are associated with books, and each book can have any number of tags.

Unlike some other systems, tags serve almost all organisational purposes. They're tags, but they also cover categories, genres, and anything else. If you care about a specific kind of metadata, define it as a tag. If you don't care, don't define it. Tags give maximum flexibility without adding a billion unnecessary fields that are functionally identical to tags. 

Tags are any set of symbols split by a space. However, some characters have special meaning. These are:

| Character | Meaning |
| --- | --- |
| `_` | Space; `tag_name` is rendered as "tag name" outside edit dialogs. |


Some characters are reserved for future use, but can still be used. However, if they're used incorrectly before they have a semantic meaning, the semantic meaning may change unexpectedly. 

The reserved characters are:

| Character | Intended use | 
| --- | --- |
| `/` | Hierarchical tags, allowing (among other things) `Genres/Sci-fi` or `SomeFranchise/SomeSubseries` |
| `:` | Not planned |

## Book metadata

