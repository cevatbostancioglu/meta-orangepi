#!/usr/bin/env bash

set -o errexit  # stop on first error
set -o xtrace  # log every step
set -o nounset  # exit when script tries to use undeclared variables

echo "Syntax check started for $0. On any syntax error script will exit"
bash -n build.sh # check and exit for syntax errors in script.
echo "Syntax is OK."

usage ()
{
    echo "usage"    
}

is_program_installed() {
    if ! [ -x "$(command -v $1)" ]; then
        echo -e "\e[1;31mRequirement "$1" is not installed!\e[0m"
        ENV_OK=0
    fi
}

do_env_check(){
    ENV_OK=1
    is_program_installed repo
    if [ $ENV_OK -eq 1 ]; then
        echo -e "\e[1;32m\nNo problems found on build environment.\n\e[0m"
    else
        echo -e "\e[1;31m\nThere are problems with the build environment!\e[0m\n"
        exit 1
    fi
}

do_fetch ()
{
    echo "do_fetch start"

    do_env_check
    echo | repo init -b ${GIT_CURRENT_BRANCH} -u ${GIT_BRANCH_URL}
    repo sync

    echo "do_fetch done"
}

do_clean ()
{
    echo "do_clean start"

    rm -rf ${BUILD_DIR}

    echo "do_clean done"
}

do_prep_host ()
{
    echo "do_prep_host start"

    set +o nounset

    source ${POKY_DIR}/oe-init-build-env build_${TARGET_ARCH}
    
    cp ${YOCTO_DIR}/conf/bblayers.conf.example ${BUILD_DIR}/conf/bblayers.conf

    cp ${YOCTO_DIR}/conf/local.conf.example ${BUILD_DIR}/conf/local.conf

    echo "do_prep_host done"
}

do_build ()
{
    echo "do_build start"

    bitbake ${IMAGE_NAME}

    echo "do_build done"
}

do_custom_build ()
{
    echo "do_custom_build start"

    bitbake -c devshell ${1}

    echo "do_custom_build done"
}

###############################################################################
# MAIN
###############################################################################

# Present usage.
if [ $# -eq 0 ]; then
    usage
    exit 0
fi

source ${PWD}/.config

SHIFTCOUNT=0

while getopts ":h?:o:f:m:p:c:i:" opt; do
    case "${opt:-}" in
        h|\?)
            usage
            exit 0
            ;;
        o)
            export BUILD_TYPE=$OPTARG
            SHIFTCOUNT=$(( $SHIFTCOUNT+2 ))
            ;;
        f)
            export IMAGE_NAME=$OPTARG
            SHIFTCOUNT=$(( $SHIFTCOUNT+2 ))
            ;;
        m)
            export MACHINE_NAME=$OPTARG
            SHIFTCOUNT=$(( $SHIFTCOUNT+2 ))
            ;;
        p)
            export IMAGE_TYPE=$OPTARG
            SHIFTCOUNT=$(( $SHIFTCOUNT+2 ))
            ;;
        c)
            export CUSTOM_RECIPE=$OPTARG
            SHIFTCOUNT=$(( $SHIFTCOUNT+2 ))
            ;;
        i)
            export TARGET_ADDRESS=$OPTARG
            SHIFTCOUNT=$(( $SHIFTCOUNT+2 ))
            ;;
    esac
done

shift $SHIFTCOUNT

# Process all commands.
while true ; do
    case "$1" in
        fetch)
            do_fetch
            shift
            break
            ;;
        clean)
            do_clean
            shift
            break
            ;;
        build)
            do_prep_host
            do_build
            shift
            break
            ;;
        custom-build)
            do_prep_host
            do_custom_build ${2}
            shift
            break
            ;;
        *)
            if [[ -n $1 ]]; then
                echo "!!!!!!!!!!!Unknown build command " $1 "!!!!!!!!!!!!!"
                usage
            fi
            shift
            break
            ;;
    esac
done