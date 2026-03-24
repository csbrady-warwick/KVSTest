# Key Value Store Test

To generate the example files run either
```
generate
```

to generate input files that do not try to order keys or blocks (the output order is hence not entirely deterministic).
This often produces the most readable files but means that it is not *possible* to have any ordering between keys.
This means that it is only possible to have hard coded ordering, and also means that duplicate keys or blocks are not possible

or

```
generateOrdered
```

to generate input files that do force an ordering on keys and blocks. It also permits duplicate blocks

Either program generates the following files in the `output` directory

* output.deck - EPOCH style file
* output.yaml - YAML file
* output.toml - TOML file
* output.json - JSON file

You can then query the file using the other two programs

## `elementAccess`

This program shows the way in which you would access blocks and keys programatically by name, including the `upsert` mechanic 
that is needed to prevent map like access from creating new keys

## `traverse`

This program shows how to traverse the structure of a parsed file in total rather than accessing it key by key
