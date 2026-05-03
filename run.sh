#!/bin/bash

# Configuration
PROGRAM_NAME="matrix_dist"
SOURCE_FILE="lab032.c" # Change this to your actual .c filename
N_SIZE=16000           # Matrix size (nxn)
NUM_SLAVES=16
START_PORT=8081

# 1. Compile the code
echo "Compiling $SOURCE_FILE..."
gcc $SOURCE_FILE -o $PROGRAM_NAME
if [ $? -ne 0 ]; then
    echo "Compilation failed!"
    exit 1
fi

# 2. Start Slaves in the background
echo "Starting $NUM_SLAVES slaves..."
for (( i=0; i<$NUM_SLAVES; i++ ))
do
    PORT=$((START_PORT + i))
    # Run slave: ./program <n> <port> <status=1>
    # ./$PROGRAM_NAME $N_SIZE $PORT 1 > "slave_$PORT.log" 2>&1 &
    ./$PROGRAM_NAME $N_SIZE $PORT 1 &

    SLAVE_PIDS[$i]=$!
done

# Give slaves a second to open their sockets
sleep 1

# 3. Start Master
echo "Starting Master..."
# Run master: ./program <n> <port (ignored by master)> <status=0>
./$PROGRAM_NAME $N_SIZE 0 0

# 4. Cleanup: Kill background slave processes after master finishes
echo "Cleaning up slaves..."
for pid in "${SLAVE_PIDS[@]}"
do
    kill $pid 2>/dev/null
done

# echo "Done. Check slave_*.log files for slave output."