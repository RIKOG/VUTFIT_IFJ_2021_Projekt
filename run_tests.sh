#!/bin/bash

RED='\033[0;31m'
NC='\033[0m'
GREEN='\033[0;32m'

name=$1
expected_err=$2
valgrind_en=$3
folder=""
code_coverage=$3
rm_out="1>/dev/null 2>/dev/null"
err_files=0
err_memory=0
all_files=0
run_lua=0
run_ifjcode=0
help=0

code_generator=0
only_names=0
in=""


error_exit() {
    echo "ERROR"
    exit 1
}

while [ "$#" -gt 0 ]; do
    case "$1" in
    *".tl")
        in="$1"
        ;;
    "--code_generator")
        code_generator=1
        ;;
    "--only_names")
        only_names=1
        ;;
    "--run_lua")
        run_lua=1
        ;;
    "--run_ifjcode")
        run_ifjcode=1
        ;;
    "--help")
        help=1
        ;;
    esac
    shift
done

if [ "${help}" -eq 1 ]; then
    echo "MACOS"
    echo "./run_tests.sh --code_generator"
    echo "UBUNTU"
    echo "./run_tests.sh --code_generator > without_errors_output.out"
    exit 1
fi

if [ "$name" == "teal_ok" ];then
    cd without_errors || exit 1
    for file in $(ls ${expected_err}*.tl); do
        echo ""
        echo "${file}"
        eval "tl run ${file}"
    done
    cd .. || exit 1
    exit 0
fi

if [ "$code_generator" -eq 1 ]; then
    eval "./start.sh --compile"
        eval "./start.sh --compile_to_ifjcode"
    if [ $(uname) == "Darwin" ];then
        eval "./start.sh --clean --compile_to_lua"
    fi

    if [ $(uname) != "Darwin" ];then
        cd without_errors || error_exit
        for file in *.tl; do
            lua_cmd="lua ${file%.*}.lua"
            ifjcode_cmd="./ic21int ${file%.*}.ifjcode"
            [[ "$run_lua"     -eq 1 ]] && ret_val_lua=$(${lua_cmd})       || ret_val_lua=""
            [[ "$run_ifjcode" -eq 1 ]] && ret_val_ifjcode=$($ifjcode_cmd) || ret_val_ifjcode=""

            if [ "${file}" == "ifj21.tl" ]; then
                continue
            fi

            if [ "$only_names" -eq 1 ]; then
                if [ "$ret_val_lua" != "$ret_val_ifjcode" ]; then
                    echo "${file}"
                fi
            else
                if [ "$ret_val_lua" != "$ret_val_ifjcode" ]; then
                    if [ "$(basename "$in")" == "${file}" ] || [ "$in" == "" ];then
                        echo ""
                        echo "#############################################################################"
                        echo "${file}"
                        echo "LUA:"
                        eval "${lua_cmd}"
                        echo "-----------------------------------------------------------------------------"
                        echo "IFJCODE:"
                        eval "${ifjcode_cmd}"
                        echo "#############################################################################"
                        echo ""
                    fi
                fi
            fi
        done
        cd .. || error_exit
    fi

    cd .. || error_exit
    exit 0
fi

code_coverage_scanner()
{
    cd build && rm -rf *
    cmake -DCOMPILE_TESTS=on -DDEBUG_SCANNER=on .. 1>/dev/null 2>/dev/null && make 1>/dev/null 2>/dev/null && ./scannerTests
    cd CMakeFiles/scannerTests.dir/src
    gcovr -b --filter ../../../../src/ --json > code1.json
}

code_coverage_other()
{
    NUM=$1
    cd build/CMakeFiles/compiler.dir/src
    gcovr -b --filter ../../../../src/ --json > code"$NUM".json
}

if [[ "$name" == "scanner_tests" ]]; then
    code_coverage_scanner

    gcovr --filter ../../../../src/ --add-tracefile code1.json --html --html-details -o code_coverage.html
    open code_coverage.html
    exit 0
fi

if [[ "$name" == "all" ]]; then
    #code_coverage_scanner
    #
    #cp code1.json ../../../../ && cd ../../../ && rm -rf *
    #cmake .. 1>/dev/null 2>/dev/null && make 1>/dev/null 2>/dev/null && cd ..

    for NUM_TEST in 0 2 3 4 5 6 7
    do
        echo "******** RET_VALUE = $NUM_TEST ********"
        ./run_tests.sh "*" "$NUM_TEST"
        echo "*******************************"
        echo ""

        code_coverage_other $NUM_TEST

        cp code"$NUM_TEST".json ../../../../ && cd ../../../../
    done

    mkdir html
    # add --add-tracefile code1.json and ucomment 41 - 44
    gcovr --filter src/ --add-tracefile code0.json \
                        --add-tracefile code2.json \
                        --add-tracefile code3.json \
                        --add-tracefile code4.json \
                        --add-tracefile code5.json \
                        --add-tracefile code6.json \
                        --add-tracefile code7.json \
                        --html --html-details -o html/code_coverage.html

    open html/code_coverage.html
    rm *.json && rm -rf html/
    exit 0
fi

cd build/ && rm -rf * && cmake .. 1>/dev/null 2>/dev/null && make 1>/dev/null 2>/dev/null && cd ..

echo ""
if [[ "$expected_err" -eq "0" ]]; then
    folder="without_errors"
elif [[ "$expected_err" -eq "2" ]]; then
    folder="syntax_errors"
elif [[ "$expected_err" -ge "3" ]] && [[ "$expected_err" -le "7" ]]; then
    folder="semantic_errors"
else
    echo "This script doesnt support this type of error $expected_error"
    exit 1
fi

for file in ${folder}/${name}*_${expected_err}.tl;
do
    if [[ "${folder}/${name}*_${expected_err}.tl" == "$file" ]]; then
        echo "Files with these arguments were not found"
        exit 1
    fi

    all_files=$((all_files+1))

    if [[ "$valgrind_en" == "valgrind" ]]; then
        valgrind --log-file="tmp.txt" build/compiler <$file 2>/dev/null 1>/dev/null
        ret_val=$?
        OUT1=$(cat tmp.txt | grep -h 'in use at exit:')
        OUT2=$(cat tmp.txt | grep -h 'errors from')
        if [[ "$OUT1" == *"0 bytes in 0 blocks"* ]]; then
            if [[ "$OUT2" != *"0 errors from 0 contexts"* ]]; then
                err_memory=$((err_memory+1))
                printf "$file ${RED}ERROR${NC} with memory\n"
                echo ""
            fi
        else
            err_memory=$((err_memory+1))
            printf "$file ${RED}ERROR${NC} with memory\n"
            echo ""
        fi

        rm tmp.txt
    else
        build/compiler <$file 2>/dev/null 1>/dev/null
        ret_val=$?
    fi

    if [ $ret_val -ne $expected_err ]; then
        err_files=$((err_files+1))
        printf "$file ${RED}FAILED${NC} ret_val = $ret_val, expected $expected_err\n"
        echo ""
    fi
done

if [[ "$err_files" -eq "0" ]] && [[ "$err_memory" -eq "0" ]] && [[ "$valgrind_en" == "valgrind" ]]; then
    printf "$all_files/$all_files tests were ${GREEN}PASSED${NC}\n"
    printf "No errors with memory\n"
elif [[ "$err_files" -eq "0" ]] && [[ "$valgrind_en" == "" ]]; then
    printf "$all_files/$all_files tests were ${GREEN}PASSED${NC}\n"
    printf "Errors with memory were not enabled\n"
else
    if [ $err_files -ne 0 ]; then
        printf "$err_files/$all_files tests were ${RED}FAILED${NC}\n"
        err_files=$((all_files-err_files))
        printf "$err_files/$all_files tests were ${GREEN}PASSED${NC}\n"
    else
        printf "$all_files/$all_files tests were ${GREEN}PASSED${NC}\n"
    fi

    if [[ "$valgrind_en" == "valgrind" ]] && [ $err_memory -ne 0 ]; then
        printf "$err_memory errors with memory was detected\n"
    elif [[ "$valgrind_en" == "valgrind" ]] && [ $err_memory -eq 0 ]; then
        printf "No errors with memory\n"
    else
        printf "Errors with memory were not enabled\n"
    fi
fi

if [[ "$code_coverage" == "code_coverage" ]]; then
    code_coverage_other 0
    gcovr --filter ../../../../src/ --add-tracefile code0.json --html --html-details -o code_coverage.html
    open code_coverage.html
fi

exit 0
