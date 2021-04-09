@echo off
cpplint --linelength=100 --recursive --filter=-runtime/references --quiet --verbose=0 ../src ../test
