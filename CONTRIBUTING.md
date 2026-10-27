# Contributing

Issues and pull requests are welcome.

- Keep changes small and focused, one topic per PR.
- `nutrition/`: run `npm test` before pushing; add a test for any new formula and cite its source in a comment.
- `firmware/`: make sure `pio run` still builds for `esp32dev`. If you touch filtering or calibration, say which load cell you tested on and how.
- No new runtime dependencies in `nutrition/` please, it's meant to stay copy-pasteable.

By contributing you agree your code is released under the MIT license.
