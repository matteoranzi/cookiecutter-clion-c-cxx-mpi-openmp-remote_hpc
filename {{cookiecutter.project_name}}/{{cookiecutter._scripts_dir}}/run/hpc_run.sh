#!/bin/bash

cd "${REMOTE_WORKING_DIRECTORY}" || { echo "[hpc_run.sh] Failed to change directory to ${REMOTE_WORKING_DIRECTORY}" >> ~/remote_run_error.log; exit 1; }


source "${HPC_CONFIG_FILE}" || { echo "[hpc_run.sh] Failed to source HPC config file ${HPC_CONFIG_FILE}" >> ~/remote_run_error.log; exit 1; }

# Preserving environment variables with -V (for example, REMOTE_WORKING_DIRECTORY)
job_id=$(qsub -V  -o /dev/null -e /dev/null -l select="${CHUNKS}":ncpus="${N_CPUS}":mem="${MEMORY}" -l walltime="${WALLTIME}" -l place="${PLACING}" -q "${QUEUE_TYPE}" "${REMOTE_WORKING_DIRECTORY}/{{cookiecutter._scripts_dir}}/run/run.pbs")  || { echo "[hpc_run.sh] Failed to submit job" >> ~/remote_run_error.log; exit 1; }
echo "JOB_ID: ${job_id}"

numeric_job_id="${job_id%%.*}"

mkdir -p "./{{cookiecutter._hpc_executions_dir}}/logs/${numeric_job_id}"
cp "${HPC_CONFIG_FILE}" "./{{cookiecutter._hpc_executions_dir}}/logs/${numeric_job_id}/job_config.log"
chmod 444 "./{{cookiecutter._hpc_executions_dir}}/logs/${numeric_job_id}/job_config.log"

mkdir -p ./{{cookiecutter._hpc_executions_dir}}/logs/last_job
echo "${numeric_job_id}" > ./{{cookiecutter._hpc_executions_dir}}/logs/last_job/job_id
