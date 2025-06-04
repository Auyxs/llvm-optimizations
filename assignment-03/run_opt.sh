#!/bin/bash

CPP_DIR="test/cpp"
BC_DIR="test/bc"
LL_DIR="test/ll"
LL_OPT_DIR="test/ll_opt"
mkdir -p "$CPP_DIR" "$BC_DIR" "$LL_DIR" "$LL_OPT_DIR"

OPT_PASS="LICM-opt"

# Get the plugin path from the environment variable
OPT_PLUGIN=${OPT_PLUGIN_PATH:-""}

if [ -z "$OPT_PLUGIN" ]; then
    echo "Error: OPT_PLUGIN_PATH environment variable is not set."
    echo "(e.g., export OPT_PLUGIN_PATH=/path/to/assignment-01/build/libLocalOpts.so)."
    exit 1
fi

for file in "$CPP_DIR"/*.cpp; do
    [ -e "$file" ] || continue  

    BASENAME=$(basename "$file" .cpp)

    # Generate initial LLVM IR (.bc)
    clang -S -emit-llvm -Xclang -disable-O0-optnone -O0 "$file" -o "$BC_DIR/$BASENAME.mem.bc"
    opt -passes=mem2reg "$BC_DIR/$BASENAME.mem.bc" -o "$BC_DIR/$BASENAME.bc"
    llvm-dis "$BC_DIR/$BASENAME.bc" -o "$LL_DIR/$BASENAME.ll"

    # Apply standard loop passes and emit intermediate LL (human-readable)
    opt -passes="loop-simplify,loop-rotate" "$BC_DIR/$BASENAME.bc" -o "$BC_DIR/$BASENAME.pre.bc"
    llvm-dis "$BC_DIR/$BASENAME.pre.bc" -o "$LL_DIR/$BASENAME.pre.ll"

    # Apply custom pass to the preprocessed IR
    opt -load-pass-plugin "$OPT_PLUGIN" -passes="$OPT_PASS" "$BC_DIR/$BASENAME.pre.bc" -o "$BC_DIR/$BASENAME.opt.bc"
    llvm-dis "$BC_DIR/$BASENAME.opt.bc" -o "$LL_OPT_DIR/$BASENAME.opt.ll"
    echo "Completed: $BASENAME"
done

echo "All files processed!"