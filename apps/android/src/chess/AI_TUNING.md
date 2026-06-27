# Cat Chess AI Tuning

The release Android build does not run the tuning harness.

To build/run the local harness manually:

```sh
apps/android/tools/run_ai_tuning.sh
```

The harness prints:

- selected move
- score
- depth reached
- nodes and quiescence nodes
- elapsed search time
- principal variation
- pass/fail result for tactical cases

Optional CMake target:

```sh
cmake -DCAT_CHESS_AI_TUNING=ON ...
```

The CMake option is `OFF` by default so release performance is unaffected.

AI evaluation weights live in `chess_ai.c` in the `chess_ai_weights` table.
