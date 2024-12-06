#!/bin/bash

args=()
for file in $@; do
    fname=`basename $file`
    args+=("$fname")
done;
./ctrapp_nl pak-o-mat.ctr "${args[@]}"
