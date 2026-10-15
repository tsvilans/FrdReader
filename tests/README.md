Run the experimental reader regression test from the repository root:

```bash
c++ -std=c++17 -Ilibfrd tests/test_reader2.cpp libfrd/frd_reader2.cpp -o /tmp/test_reader2
timeout 10 /tmp/test_reader2 /tmp/test_reader2.frd
```

The test covers mesh parsing, terminating on unsupported records, bounded
result-header buffers, indexed scalar lookups, batch output propagation,
out-of-order node requests, and failed lookups without partial output. The
experimental reader's compact `data` layout is initialized explicitly for the
lookup tests; constructing that layout from an FRD file remains unfinished.
