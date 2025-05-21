# Search

## Possible search index candidates

* https://github.com/valeriansaliou/sonic
    * Might be embeddable with a lot of nasty hacks
    * The client needs to be ported in from rust as well. No C++ clients appear to exist
        * If the server is embedded, maybe it can be accessed directly without needing special client bindings?

## Embedding vs. standalone

So far, it looks like Sonic is the only real option that isn't OpenSearch/ElasticSearch-levels of heavy and complex. Considering how light it is, it might be defensible to leave it as a separate service. Not sure how tedious it is to set up compared to PSQL though.
