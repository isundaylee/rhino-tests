#!/bin/bash

WORKSPACE=workspace
LOGSPACE="../logs"

BIN_RHINO=$HOME/rhino/build/bin
CC_RHINO="$BIN_RHINO/clang -ftapir=cilk"
CXX_RHINO="$BIN_RHINO/clang++ -ftapir=cilk"

LOG_TAG=""

BENCHMARKS="AveragingFilter_01_07_15 BlackScholes_12_17_14 Mandelbrot_12_17_14 ShortestPath_12_31_14"
NUM_WORKERS="1 2 4 8"
TRIALS=5

# COMPILE ERRORS:
# MonteCarloSample_12_17_14 Rtm_Stencil_12_31_14 SepiaFilter_01_07_15
# WRONG RUNTIME EXTRACTION:
# BinomialLattice_12_31_14 DCT_01_07_15

function log_set_tag() {
    LOG_TAG=$1
}

function log() {
    printf "[%s] %s\n" "$LOG_TAG" "$1"
}

function controlled_run() {
    num_workers=$1
    log=$2
    shift 2

    taskset -c 0-$(($num_workers-1)) numactl -i all "$@" >$log 2>&1
}

function build_and_run_intel() {
    # Remove and re-create workspace
    rm -r $WORKSPACE
    mkdir $WORKSPACE

    mkdir -p $WORKSPACE/$LOGSPACE

    log_set_tag "$1"
    log "Copying benchmark files..."
    cp -r suites/intel/$1/* $WORKSPACE/

    pushd $WORKSPACE > /dev/null
    build_intel "$1"
    run_intel "$1" "$2" "$3"
    popd > /dev/null
}

function build_intel() {
    log_build="$LOGSPACE/$1-build.log"

    export CC=$CC_RHINO
    export CXX=$CXX_RHINO
    log "Building (log at $WORKSPACE/$log_build)..."
    make run -B perf_num=1 >$log_build 2>&1
    unset CC
    unset CXX
}

function extract_runtime_intel() {
    runtime=$(tail -1 $1)

    if [[ $runtime = *"ms"* ]]; then
        runtime=$(echo $runtime | cut -d' ' -f3 | tr -d ms)
        runtime=$(echo "print $runtime/1000.0" | python)
    fi

    printf "%.6f" $runtime
}

function run_intel() {
    for num_workers in $2; do
        log "Running benchmark with $num_workers workers..."

        export CILK_NWORKERS=$num_workers

        for trial in `seq 1 $3`; do
            log_run="$LOGSPACE/$1-run-$num_workers-$trial.log"
            controlled_run $num_workers $log_run make run perf_num=1

            runtime=$(extract_runtime_intel $log_run)
            log "Running (log at $WORKSPACE/$log_run)... Runtime: $runtime s"
        done

        unset CILK_NWORKERS
    done
}

rm $WORKSPACE/$LOGSPACE/*.log

for benchmark in $BENCHMARKS; do
    build_and_run_intel $benchmark "$NUM_WORKERS" $TRIALS
done
