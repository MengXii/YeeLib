#!/usr/bin/env bash
DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"
MINICONDA_PREFIX="${HOME}/miniconda3"
CONDA_URL=${CONDA_URL:-https://repo.anaconda.com}

set +e

if [ ! -d "${MINICONDA_PREFIX}" ]; then
  (>&2 echo "Setting up conda binary (miniconda)")
  TMP_INSTALLER="/tmp/Miniconda3-latest-Linux-x86_64.sh"
  if [ ! -f "${TMP_INSTALLER}" ]; then
    curl -L -o "${TMP_INSTALLER}" ${CONDA_URL}/miniconda/Miniconda3-latest-Linux-x86_64.sh
  fi
  bash "${TMP_INSTALLER}" -b -p "${MINICONDA_PREFIX}" -f
else
  (>&2 echo "Skip setting up conda binary (miniconda)")
fi

source "${MINICONDA_PREFIX}/etc/profile.d/conda.sh"

if conda config --get channels | grep -q "${CACHE_URL}"; then
  conda config --remove channels "${CACHE_URL}"
fi

(>&2 echo "Updating conda binary (miniconda)")
conda config --set auto_activate_base false
conda update -y -n base -c defaults conda
conda install conda-build -y

RUN_CONDA="source \$HOME/miniconda3/etc/profile.d/conda.sh"
cat ~/.bashrc | grep "$RUN_CONDA" &> /dev/null
if [ $? != 0 ]; then
  (>&2 echo "Added init script to .bashrc")
  echo $RUN_CONDA >> ~/.bashrc
else
  (>&2 echo "The init script is already in .bashrc")
fi