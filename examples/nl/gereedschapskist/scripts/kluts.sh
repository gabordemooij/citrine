#!/bin/bash
echo $1 | sha256sum | cut -d " " -f 1

